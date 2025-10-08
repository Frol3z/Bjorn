#include "Application.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <filesystem>

// Validation layers
#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif

const std::vector validationLayers = // and FPS monitoring layer
{
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_LUNARG_monitor"
};

// Note that for greater number of concurrent frames
// the CPU might get ahead of the GPU causing latency
// between frames
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace Bjorn 
{
	Renderer::Renderer(Application& app, const Window& window, const Scene& scene)
        : m_app(app), m_window(window), m_scene(scene)
    {
        CreateInstance();
        CreateSurface();
        SelectPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapchain();
        CreateGraphicsPipeline();
        CreateCommandPool();
        CreateVertexBuffer();
        CreateCommandBuffer();
        CreateSyncObjects();
    }

    void Renderer::DrawFrame()
    {
        // CPU will wait until the GPU finishes rendering the previous frame (corresponding to the same swapchain image index)
        while (vk::Result::eTimeout == m_device.waitForFences(*m_inFlightFences[m_currentFrame], vk::True, UINT64_MAX));

        // Wait for the presentation to release the semaphore
        m_presentQueue.waitIdle();

        // Check if window has been resized/minimize before trying to acquire next image
        if (m_app.isFramebufferResized.load()) {
            m_app.isFramebufferResized.store(false);
            m_swapchain->RecreateSwapchain();
            return;
        }

        // Acquire next image index and signal the semaphore when done
        auto [result, imageIndex] = m_swapchain->AcquireNextImage(*m_imageAvailableSemaphores[m_currentFrame]);

        // Check if the surface is still compatible with the swapchain or if it was resized/minimized
        // !!! Checking m_IsFramebufferResized guarantees prevents presentation to an invalid surface
        if (result == vk::Result::eErrorOutOfDateKHR || m_app.isFramebufferResized.load()) {
            m_app.isFramebufferResized.store(false);
            m_swapchain->RecreateSwapchain();
            return;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        // Record command buffer and reset draw fence
        m_device.resetFences(*m_inFlightFences[m_currentFrame]);
        RecordCommandBuffer(imageIndex);

        // Submit commands to the queue
        vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        const vk::SubmitInfo submitInfo{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &*m_imageAvailableSemaphores[m_currentFrame],
            .pWaitDstStageMask = &waitDestinationStageMask,
            .commandBufferCount = 1,
            .pCommandBuffers = &*m_commandBuffers[m_currentFrame],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &*m_renderFinishedSemaphores[m_currentFrame]
        };
        m_graphicsQueue.submit(submitInfo, *m_inFlightFences[m_currentFrame]);

        // Present to the screen
        const vk::PresentInfoKHR presentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &*m_renderFinishedSemaphores[m_currentFrame],
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain->GetHandle(),
            .pImageIndices = &imageIndex
        };
        result = m_presentQueue.presentKHR(presentInfoKHR);

        // Check again if presentation fails because the surface is now incompatible
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_app.isFramebufferResized.load()) {
            m_app.isFramebufferResized.store(false);
            m_swapchain->RecreateSwapchain();
            return;
        }
        else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::WaitIdle()
    {
        m_device.waitIdle();
    }

	void Renderer::CreateInstance() 
    {
        // Optional appInfo struct
        const vk::ApplicationInfo appInfo{
            .pApplicationName = m_app.GetName().c_str(),
            .applicationVersion = VK_MAKE_VERSION(1,0,0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1,0,0),
            .apiVersion = vk::ApiVersion14
        };

        // List available instance layers and extensions
        auto layerProperties = m_context.enumerateInstanceLayerProperties();
        auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
       
        #ifdef _DEBUG
            std::cout << "Available instance layer:" << std::endl;
            for (const auto& layer : layerProperties) {
                std::cout << '\t' << layer.layerName << std::endl;
            }

            std::cout << "Available instance extensions:" << std::endl;
            for (const auto& extension : extensionProperties) {
                std::cout << '\t' << extension.extensionName << std::endl;
            }
        #endif

        // Get the required instance layers
        std::vector<char const*> requiredLayers;
        // Validation layers (optional)
        if (enableValidationLayers) {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }
        
        // ADD HERE OTHER INSTANCE LAYERS IF NEEDED
        
        // Check if the required layers are supported by the Vulkan implementation
        if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
            return std::ranges::none_of(layerProperties,
                [requiredLayer](auto const& layerProperty)
                { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
            }))
        {
            throw std::runtime_error("One or more required layers are not supported!");
        }

        // Get the required instance extensions from GLFW
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        //std::cout << glfwExtensionCount << std::endl; = 2
        
        // ADD HERE OTHER INSTANCE EXTENSIONS IF NEEDED
        
        // Check if the required GLFW extensions are supported by the Vulkan implementation
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            if (std::ranges::none_of(extensionProperties,
                [glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
                { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
            {
                throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
            }
        }

        // Vulkan instance creation
        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames = requiredLayers.data(),
            .enabledExtensionCount = glfwExtensionCount,
            .ppEnabledExtensionNames = glfwExtensions
        };

        try {
            m_instance = vk::raii::Instance(m_context, createInfo);
        }
        catch (const vk::SystemError& err) {
            std::cerr << "Vulkan error: " << err.what() << std::endl;
        }
    }

    void Renderer::CreateSurface() 
    {
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(*m_instance, m_window.GetHandle(), nullptr, &_surface) != 0) {
            throw std::runtime_error("Failed to create window surface!");
        }
        m_surface = vk::raii::SurfaceKHR(m_instance, _surface);
    }

    void Renderer::SelectPhysicalDevice() 
    {
        auto devices = m_instance.enumeratePhysicalDevices();
        if (devices.empty()) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        // List available devices
        #ifdef _DEBUG
            std::cout << "Available devices:" << std::endl;
            for (const auto& device : devices) {
                // If it is useful to query for raytracing property keep using VkPhysicalDeviceProperties2
                // else revert to VkPhysicalDeviceProperties
                auto deviceProperties = device.getProperties2();

                // Other queriable stuff if ever needed
                //auto deviceFeatures = device.getFeatures2();
                //auto extensions = device.enumerateDeviceExtensionProperties();
                //auto queueFamilyProperties = device.getQueueFamilyProperties();

                // Device properties
                std::cout << '\t' << "- " << deviceProperties.properties.deviceName << std::endl;
                //std::cout << '\t' << "API version: " << deviceProperties.properties.apiVersion << std::endl;
                //std::cout << '\t' << "Driver version: " << deviceProperties.properties.driverVersion << std::endl;
                //std::cout << '\t' << "Vendor ID: " << deviceProperties.properties.vendorID << std::endl;
                //std::cout << '\t' << "ID: " << deviceProperties.properties.deviceID << std::endl;
                /*
                switch (deviceProperties.properties.deviceType) {
                    case vk::PhysicalDeviceType::eOther: {
                        std::cout << '\t' << "Type: " << "Other" << std::endl;
                        break;
                    }
                    case vk::PhysicalDeviceType::eIntegratedGpu: {
                        std::cout << '\t' << "Type: " << "Integrated GPU" << std::endl;
                        break;
                    }
                    case vk::PhysicalDeviceType::eDiscreteGpu: {
                        std::cout << '\t' << "Type: " << "Discrete GPU" << std::endl;
                        break;
                    }
                    case vk::PhysicalDeviceType::eVirtualGpu: {
                        std::cout << '\t' << "Type: " << "Virtual GPU" << std::endl;
                        break;
                    }
                    case vk::PhysicalDeviceType::eCpu: {
                        std::cout << '\t' << "Type: " << "CPU" << std::endl;
                        break;
                    }
                }
                */
                /*
                    std::cout << '\t' << "Supported extensions:" << std::endl;
                    for (const auto& extension : extensions) {
                        std::cout << "\t\t - " << extension.extensionName << std::endl;
                    }
                */
            #endif

            // Check for suitability (TODO: properly check for raytracing extensions support)
            bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_4;
            if (isSuitable) {
                m_physicalDevice = device;
                break;
            }
        }
    }

    void Renderer::CreateLogicalDevice() 
    {
        // Get index of the first queue family with graphics support
        auto queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();
        auto graphicsQueueFamilyProperty = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
            [](vk::QueueFamilyProperties const& qfp) {
                return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
            });
        auto graphicsQueueFamilyIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

        // Check queue family for presentation support
        auto presentQueueFamilyIndex = m_physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, *m_surface) ? graphicsQueueFamilyIndex : -1;
        // TODO: I should handle when this happens
        if (presentQueueFamilyIndex == -1) {
            std::runtime_error("Present and graphics are not supported by the same queue family!");
        }

        // DeviceQueueCreateInfo
        float queuePriority = 0.0;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
            .queueFamilyIndex = graphicsQueueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        // Create a chain of feature structures to enable multiple new features (on top of those of VK 1.0) all at once
        vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> // To be able to change dinamically some pipeline properties
            featureChain = {
                {},
                {.synchronization2 = true, .dynamicRendering = true},
                {.extendedDynamicState = true}
        };

        // Required device extensions
        // TODO: actually check for support and don't rely on the validation layer
        std::vector<const char*> deviceExtensions = {
            // Mandatory extension for presenting framebuffer on a window
            vk::KHRSwapchainExtensionName,

            // Vulkan tutorial extensions (I guess I'll understand what they are here for, at some point)
            vk::KHRSpirv14ExtensionName,
            vk::KHRSynchronization2ExtensionName,
            vk::KHRCreateRenderpass2ExtensionName,
            vk::KHRShaderDrawParametersExtensionName, // Required to be able to use SV_VertexID in shader code

            // Raytracing extensions
            vk::KHRAccelerationStructureExtensionName,
            vk::KHRDeferredHostOperationsExtensionName, // Required by the extension above
            vk::KHRRayTracingPipelineExtensionName,
            vk::KHRRayQueryExtensionName
        };

        // Device creation
        vk::DeviceCreateInfo deviceCreateInfo{
            .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &deviceQueueCreateInfo,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
        };

        try {
            m_device = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
        }
        catch (const vk::SystemError& err) {
            std::cerr << "Vulkan error: " << err.what() << std::endl;
        }

        // Get graphics and presentation queue reference
        m_graphicsQueue = vk::raii::Queue(m_device, graphicsQueueFamilyIndex, 0);
        m_presentQueue = vk::raii::Queue(m_device, presentQueueFamilyIndex, 0);
        m_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    }

    void Renderer::CreateSwapchain()
    {
        m_swapchain = std::make_unique<Swapchain>(m_physicalDevice, m_device, m_window, m_surface);
    }

    void Renderer::CreateGraphicsPipeline()
    {
        // Create shader module
        auto shaderPath = std::filesystem::current_path() / "shaders/slang.spv";
        vk::raii::ShaderModule shaderModule = CreateShaderModule(ReadFile(shaderPath.string()));

        // ShaderStageCreateInfo
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = shaderModule,
            .pName = "vertMain"
        };
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = shaderModule,
            .pName = "fragMain"
        };
        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // VertexInput
        // TODO: add index buffers support
        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = attributeDescriptions.size(),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        };

        // InputAssembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
            .topology = vk::PrimitiveTopology::eTriangleList
        };

        /*
        // Pipeline-baked viewport and scissors
        vk::Viewport{
            0.0f,
            0.0f,
            static_cast<float>(m_SwapChainExtent.width),
            static_cast<float>(m_SwapChainExtent.height),
            0.0f,
            1.0f
        };

        vk::Rect2D{
            vk::Offset2D{ 0, 0 },
            m_SwapChainExtent
        };
        */

        // Dynamic viewport and scissors (will be set in the command buffer)
        std::vector dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        vk::PipelineDynamicStateCreateInfo dynamicState{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };
        // We just need to specify how many there are at pipeline creation time
        vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .scissorCount = 1 };

        // Rasterizer
        vk::PipelineRasterizationStateCreateInfo rasterizer{
            .depthClampEnable = vk::False, // Clamp out of bound depth values to the near or far plane
            .rasterizerDiscardEnable = vk::False, // Disable rasterization (no output)
            .polygonMode = vk::PolygonMode::eFill, // For wireframe mode a GPU feature must be enabled
            .cullMode = vk::CullModeFlagBits::eBack, // Self explanatory
            .frontFace = vk::FrontFace::eClockwise,
            .depthBiasEnable = vk::False,
            .depthBiasSlopeFactor = 1.0f,
            .lineWidth = 1.0f
        };

        // MSAA (currently disabled because it requires enabling a GPU feature)
        vk::PipelineMultisampleStateCreateInfo multisampling{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = vk::False
        };

        // Color blending (currently alpha blending is configured for reference but disabled)
        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = vk::False;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        // Blending with bitwise operations
        vk::PipelineColorBlendStateCreateInfo colorBlending{
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment
        };

        // Uniforms (not used yet)
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0
        };
        m_pipelineLayout = vk::raii::PipelineLayout(m_device, pipelineLayoutInfo);

        // PipelineRenderingCreateInfo
        // It specifies the formats of the attachment used with dynamic rendering
        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &m_swapchain->GetSurfaceFormat().format
        };

        // GraphicsPipelineCreateInfo
        vk::GraphicsPipelineCreateInfo pipelineInfo{
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = m_pipelineLayout,
            .renderPass = nullptr // Because we are using dynamic rendering
        };

        // FINALLY!
        m_graphicsPipeline = vk::raii::Pipeline(m_device, nullptr, pipelineInfo);
    }

    vk::raii::ShaderModule Renderer::CreateShaderModule(const std::vector<char>& code) const
    {
        vk::ShaderModuleCreateInfo createInfo{
            .codeSize = code.size() * sizeof(char),
            .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };
        vk::raii::ShaderModule shaderModule(m_device, createInfo);
        return shaderModule;
    }

    std::vector<char> Renderer::ReadFile(const std::string& filename) 
    {
        // Read starting from the end to determine the size of the file
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file!");
        }

        std::vector<char> buffer(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        file.close();

        return buffer;
    }

    void Renderer::CreateCommandPool()
    {
        // CommandPoolCreateInfo
        vk::CommandPoolCreateInfo poolInfo = {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = m_graphicsQueueFamilyIndex
        };
        m_commandPool = vk::raii::CommandPool(m_device, poolInfo);
    }

    void Renderer::CreateVertexBuffer()
    {
        // Buffer creation (with no memory assigned to it yet)
        auto vertices = m_scene.GetVertices();
        vk::BufferCreateInfo bufferInfo{
            .size = sizeof(vertices[0]) * vertices.size(),
            .usage = vk::BufferUsageFlagBits::eVertexBuffer,
            .sharingMode = vk::SharingMode::eExclusive
        };
        m_vertexBuffer = vk::raii::Buffer(m_device, bufferInfo);

        // Buffer memory allocation and binding
        vk::MemoryRequirements memRequirements = m_vertexBuffer.getMemoryRequirements();
        vk::MemoryAllocateInfo memoryAllocateInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = FindMemoryType(
                memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            )
        };
        m_vertexBufferMemory = vk::raii::DeviceMemory(m_device, memoryAllocateInfo);
        m_vertexBuffer.bindMemory(*m_vertexBufferMemory, 0);

        // Filling the GPU buffer with CPU data
        void* data = m_vertexBufferMemory.mapMemory(0, bufferInfo.size);
        memcpy(data, m_scene.GetVertices().data(), bufferInfo.size);
        m_vertexBufferMemory.unmapMemory();
    }

    void Renderer::CreateCommandBuffer()
    {
        // CommandBufferAllocateInfo
        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool = m_commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = MAX_FRAMES_IN_FLIGHT
        };
        m_commandBuffers = vk::raii::CommandBuffers(m_device, allocInfo);
    }

    void Renderer::CreateSyncObjects()
    {
        m_imageAvailableSemaphores.clear();
        m_renderFinishedSemaphores.clear();
        m_inFlightFences.clear();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_imageAvailableSemaphores.emplace_back(m_device, vk::SemaphoreCreateInfo());
            m_renderFinishedSemaphores.emplace_back(m_device, vk::SemaphoreCreateInfo());
            m_inFlightFences.emplace_back(m_device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
        }
    }

    void Renderer::RecordCommandBuffer(uint32_t imageIndex)
    {
        // Reset command buffer
        m_commandBuffers[m_currentFrame].begin({});

        // Transition image layout to color attachment
        TransitionImageLayout(
            imageIndex,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );

        // Setup color attachment
        vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        vk::RenderingAttachmentInfo attachmentInfo{
            .imageView = m_swapchain->GetImageViews()[imageIndex],
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = clearColor
        };

        // Setup rendering info
        auto swapchainExtent = m_swapchain->GetExtent();
        vk::RenderingInfo renderingInfo = {
            .renderArea = {.offset = { 0, 0 }, .extent = swapchainExtent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentInfo
        };

        // Begin rendering
        m_commandBuffers[m_currentFrame].beginRendering(renderingInfo);

        // Bind the graphic pipeline (the attachment will be bound to the fragment shader output)
        m_commandBuffers[m_currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);

        // Set viewport and scissor size (dynamic rendering)
        m_commandBuffers[m_currentFrame].setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchainExtent.width), static_cast<float>(swapchainExtent.height), 0.0f, 1.0f));
        m_commandBuffers[m_currentFrame].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchainExtent));

        // Bind vertex buffer
        m_commandBuffers[m_currentFrame].bindVertexBuffers(0, *m_vertexBuffer, { 0 });

        // Draw command
        m_commandBuffers[m_currentFrame].draw(3, 1, 0, 0);

        // Finish up rendering
        m_commandBuffers[m_currentFrame].endRendering();

        // After rendering, transition the swapchain image to PRESENT_SRC
        TransitionImageLayout(
            imageIndex,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe
        );

        // Finish recording command onto the buffer
        m_commandBuffers[m_currentFrame].end();
    }

    void Renderer::TransitionImageLayout(
        uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    )
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = srcStageMask,
            .srcAccessMask = srcAccessMask,
            .dstStageMask = dstStageMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_swapchain->GetImages()[imageIndex],
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };
        m_commandBuffers[m_currentFrame].pipelineBarrier2(dependencyInfo);
    }

    uint32_t Renderer::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            // Check for memory type and properties suitability
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type!");
    }
}