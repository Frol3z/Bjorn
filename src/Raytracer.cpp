#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

// Note that for greater number of concurrent frames
// the CPU might get ahead of the GPU causing latency
// between frames
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

// TODO: Refactor class in dedicated header file and cpp file
class Application {
public:
    void Run() {
        Init();
        MainLoop();
        CleanUp();
    }
private:
    void Init() {
        InitWindow();
        InitVulkan();
    }

    void MainLoop() {
        while (!glfwWindowShouldClose(m_Window)) {
            glfwPollEvents();
            DrawFrame();
        }

        // Wait for pending GPU operations to finish
        m_Device.waitIdle();
    }

    void CleanUp() {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void DrawFrame() {
        // CPU will wait until the GPU finishes rendering the previous frame (corresponding to the same swapchain image index)
        while (vk::Result::eTimeout == m_Device.waitForFences(*m_InFlightFences[m_CurrentFrame], vk::True, UINT64_MAX));

        // Wait for the presentation to release the semaphore
        m_PresentQueue.waitIdle();

        // Check if window has been resized/minimize before trying to acquire next image
        if (m_IsFramebufferResized) {
            m_IsFramebufferResized = false;
            RecreateSwapChain();
            return;
        }

        // Acquire next image index and signal the semaphore when done
        auto [result, imageIndex] = m_SwapChain.acquireNextImage(UINT64_MAX, *m_ImageAvailableSemaphores[m_CurrentFrame], nullptr);

        // Check if the surface is still compatible with the swapchain or if it was resized/minimized
        // !!! Checking m_IsFramebufferResized guarantees prevents presentation to an invalid surface
        if (result == vk::Result::eErrorOutOfDateKHR || m_IsFramebufferResized) {
            m_IsFramebufferResized = false;
            RecreateSwapChain();
            return;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        // Record command buffer and reset draw fence
        m_Device.resetFences(*m_InFlightFences[m_CurrentFrame]);
        RecordCommandBuffer(imageIndex);

        // Submit commands to the queue
        vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        const vk::SubmitInfo submitInfo{ 
            .waitSemaphoreCount = 1, 
            .pWaitSemaphores = &*m_ImageAvailableSemaphores[m_CurrentFrame],
            .pWaitDstStageMask = &waitDestinationStageMask, 
            .commandBufferCount = 1, 
            .pCommandBuffers = &*m_CommandBuffers[m_CurrentFrame],
            .signalSemaphoreCount = 1, 
            .pSignalSemaphores = &*m_RenderFinishedSemaphores[m_CurrentFrame]
        };
        m_GraphicsQueue.submit(submitInfo, *m_InFlightFences[m_CurrentFrame]);

        // Present to the screen
        const vk::PresentInfoKHR presentInfoKHR{ 
            .waitSemaphoreCount = 1, 
            .pWaitSemaphores = &*m_RenderFinishedSemaphores[m_CurrentFrame],
            .swapchainCount = 1, 
            .pSwapchains = &*m_SwapChain, 
            .pImageIndices = &imageIndex
        };
        result = m_PresentQueue.presentKHR(presentInfoKHR);

        // Check again if presentation fails because the surface is now incompatible
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_IsFramebufferResized) {
            m_IsFramebufferResized = false;
            RecreateSwapChain();
            return;
        }
        else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void InitWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Application", nullptr, nullptr);


        glfwSetWindowUserPointer(m_Window, this);
        // Set callback function when the window gets resized
        glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
    }

    void InitVulkan() {
        CreateInstance();
        CreateSurface();
        SelectPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateImageViews();
        CreateGraphicsPipeline();
        CreateCommandPool();
        CreateCommandBuffer();
        CreateSyncObjects();
    }

    void CreateInstance() {
        // Optional appInfo struct
        constexpr vk::ApplicationInfo appInfo{
            .pApplicationName = "Application",
            .applicationVersion = VK_MAKE_VERSION(1,0,0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1,0,0),
            .apiVersion = vk::ApiVersion14
        };

        // TODO: move extensions and layers listing to dedicated functions that can be turned on and off
        // List available instance layers
        auto layerProperties = m_Context.enumerateInstanceLayerProperties();
        std::cout << "Available instance layer:" << std::endl;
        for (const auto& layer : layerProperties) {
            std::cout << '\t' << layer.layerName << std::endl;
        }

        // List available instance extensions
        auto extensionProperties = m_Context.enumerateInstanceExtensionProperties();
        std::cout << "Available instance extensions:" << std::endl;
        for (const auto& extension : extensionProperties) {
            std::cout << '\t' << extension.extensionName << std::endl;
        }

        // Get the required instance layers
        std::vector<char const*> requiredLayers;
        if (enableValidationLayers) {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }

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
            m_Instance = vk::raii::Instance(m_Context, createInfo);
        }
        catch (const vk::SystemError& err) {
            std::cerr << "Vulkan error: " << err.what() << std::endl;
        }
    }

    void CreateSurface() {
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(*m_Instance, m_Window, nullptr, &_surface) != 0) {
            throw std::runtime_error("Failed to create window surface!");
        }
        m_Surface = vk::raii::SurfaceKHR(m_Instance, _surface);
    }

    void SelectPhysicalDevice() {
        auto devices = m_Instance.enumeratePhysicalDevices();        
        
        if (devices.empty()) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
        
        // List available devices
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
            std::cout << '\t' << "Name: " << deviceProperties.properties.deviceName << std::endl;
            std::cout << '\t' << "API version: " << deviceProperties.properties.apiVersion << std::endl;
            std::cout << '\t' << "Driver version: " << deviceProperties.properties.driverVersion << std::endl;
            std::cout << '\t' << "Vendor ID: " << deviceProperties.properties.vendorID << std::endl;
            std::cout << '\t' << "ID: " << deviceProperties.properties.deviceID << std::endl;

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
            
            /*
                std::cout << '\t' << "Supported extensions:" << std::endl;
                for (const auto& extension : extensions) {
                    std::cout << "\t\t - " << extension.extensionName << std::endl;
                }
            */
            
            // Check for suitability (TODO: properly check for raytracing extensions support)
            bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_4;
            if (isSuitable) {
                m_PhysicalDevice = device;
                break;
            }
        }
    }

    void CreateLogicalDevice() {
        // Get index of the first queue family with graphics support
        auto queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();
        auto graphicsQueueFamilyProperty = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
            [](vk::QueueFamilyProperties const& qfp) {
                return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
            });
        auto graphicsQueueFamilyIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

        // Check queue family for presentation support
        auto presentQueueFamilyIndex = m_PhysicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, *m_Surface) ? graphicsQueueFamilyIndex : -1;
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
            m_Device = vk::raii::Device(m_PhysicalDevice, deviceCreateInfo);
        }
        catch (const vk::SystemError& err) {
            std::cerr << "Vulkan error: " << err.what() << std::endl;
        }

        // Get graphics and presentation queue reference
        m_GraphicsQueue = vk::raii::Queue(m_Device, graphicsQueueFamilyIndex, 0);
        m_PresentQueue = vk::raii::Queue(m_Device, presentQueueFamilyIndex, 0);
        m_GraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    }

    void CreateSwapChain() {
        // Query for surface capabilities (min/max number of images, min/max width and height)
        auto surfaceCapabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface);
        // Query for surface format (pixel format, color space)
        std::vector<vk::SurfaceFormatKHR> availableFormats = m_PhysicalDevice.getSurfaceFormatsKHR(m_Surface);
        // Query for available presentation modes
        std::vector<vk::PresentModeKHR> availablePresentModes = m_PhysicalDevice.getSurfacePresentModesKHR(m_Surface);

        // Choose suitable surface format and extent
        m_SwapChainSurfaceFormat = ChooseSwapChainSurfaceFormat(availableFormats);
        m_SwapChainExtent = ChooseSwapChainExtent(surfaceCapabilities);

        // Choose how many images to have in the swap chain
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        minImageCount = (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) 
            ? surfaceCapabilities.maxImageCount
            : minImageCount;

        // SwapChainCreateInfo
        // If rendering to a separate image (e.g. to apply post-processing effects) and then
        // presenting to the screen imageUsage should be set to VK_IMAGE_USAGE_TRANSFER_DST_BIT
        vk::SwapchainCreateInfoKHR swapChainCreateInfo{
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = m_Surface, 
            .minImageCount = minImageCount,
            .imageFormat = m_SwapChainSurfaceFormat.format, 
            .imageColorSpace = m_SwapChainSurfaceFormat.colorSpace,
            .imageExtent = m_SwapChainExtent, 
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive, // Assuming graphics and presentation queue family is the same
            .preTransform = surfaceCapabilities.currentTransform, 
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = ChooseSwapChainPresentMode(availablePresentModes),
            .clipped = true, 
            .oldSwapchain = nullptr 
        };

        // Create swap chain
        m_SwapChain = vk::raii::SwapchainKHR(m_Device, swapChainCreateInfo);
        m_SwapChainImages = m_SwapChain.getImages();
    }

    void RecreateSwapChain() {
        // Handling minimization
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_Window, &width, &height);
            glfwWaitEvents();
        }
        
        m_Device.waitIdle();

        CleanUpSwapChain();

        CreateSwapChain();
        CreateImageViews();
    }

    void CleanUpSwapChain() {
        m_SwapChainImageViews.clear();
        m_SwapChain = nullptr;
    }

    void CreateImageViews() {
        m_SwapChainImageViews.clear();

        // ImageViewCreateInfo
        vk::ImageViewCreateInfo imageViewCreateInfo{
            .viewType = vk::ImageViewType::e2D,
            .format = m_SwapChainSurfaceFormat.format,
            .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        };

        // Create image views for each image
        for (auto image : m_SwapChainImages) {
            imageViewCreateInfo.image = image;
            m_SwapChainImageViews.emplace_back(m_Device, imageViewCreateInfo);
        }
    }

    void CreateGraphicsPipeline() {
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
        // TODO: remove hardcoded vertices and provide vertex and index buffers
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

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
        m_PipelineLayout = vk::raii::PipelineLayout(m_Device, pipelineLayoutInfo);

        // PipelineRenderingCreateInfo
        // It specifies the formats of the attachment used with dynamic rendering
        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ 
            .colorAttachmentCount = 1, 
            .pColorAttachmentFormats = &m_SwapChainSurfaceFormat.format 
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
            .layout = m_PipelineLayout, 
            .renderPass = nullptr // Because we are using dynamic rendering
        };

        // FINALLY!
        m_GraphicsPipeline = vk::raii::Pipeline(m_Device, nullptr, pipelineInfo);
    }

    void CreateCommandPool() {
        // CommandPoolCreateInfo
        vk::CommandPoolCreateInfo poolInfo = {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = m_GraphicsQueueFamilyIndex
        };
        m_CommandPool = vk::raii::CommandPool(m_Device, poolInfo);
    }

    void CreateCommandBuffer() {
        // CommandBufferAllocateInfo
        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool = m_CommandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = MAX_FRAMES_IN_FLIGHT
        };
        m_CommandBuffers = vk::raii::CommandBuffers(m_Device, allocInfo);
    }

    void CreateSyncObjects() {
        m_ImageAvailableSemaphores.clear();
        m_RenderFinishedSemaphores.clear();
        m_InFlightFences.clear();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_ImageAvailableSemaphores.emplace_back(m_Device, vk::SemaphoreCreateInfo());
            m_RenderFinishedSemaphores.emplace_back(m_Device, vk::SemaphoreCreateInfo());
            m_InFlightFences.emplace_back(m_Device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
        }
    }

    // SwapChain stuff
    vk::SurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    vk::PresentModeKHR ChooseSwapChainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        // The only mode guaranteed to be available (may rasult in visible tearing)
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D ChooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        int width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);
        return {
            std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }

    // Command buffer stuff
    void RecordCommandBuffer(uint32_t imageIndex) {
        // Reset command buffer
        m_CommandBuffers[m_CurrentFrame].begin({});

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
        vk::ClearValue clearColor = vk::ClearColorValue(0.98f, 0.93f, 0.93f, 1.0f);
        vk::RenderingAttachmentInfo attachmentInfo{
            .imageView = m_SwapChainImageViews[imageIndex],
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = clearColor
        };

        // Setup rendering info
        vk::RenderingInfo renderingInfo = {
            .renderArea = {.offset = { 0, 0 }, .extent = m_SwapChainExtent },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentInfo
        };

        // Begin rendering
        m_CommandBuffers[m_CurrentFrame].beginRendering(renderingInfo);

        // Bind the graphic pipeline (the attachment will be bound to the fragment shader output)
        m_CommandBuffers[m_CurrentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicsPipeline);

        // Set viewport and scissor size (dynamic rendering)
        m_CommandBuffers[m_CurrentFrame].setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(m_SwapChainExtent.width), static_cast<float>(m_SwapChainExtent.height), 0.0f, 1.0f));
        m_CommandBuffers[m_CurrentFrame].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_SwapChainExtent));

        // Draw command
        m_CommandBuffers[m_CurrentFrame].draw(3, 1, 0, 0);

        // Finish up rendering
        m_CommandBuffers[m_CurrentFrame].endRendering();

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
        m_CommandBuffers[m_CurrentFrame].end();
    }

    // Utilities
    static std::vector<char> ReadFile(const std::string& filename) {
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

    [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code) const {
        vk::ShaderModuleCreateInfo createInfo{
            .codeSize = code.size() * sizeof(char),
            .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };
        vk::raii::ShaderModule shaderModule(m_Device, createInfo);
        return shaderModule;
    }

    void TransitionImageLayout(
        uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    ) {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = srcStageMask,
            .srcAccessMask = srcAccessMask,
            .dstStageMask = dstStageMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_SwapChainImages[imageIndex],
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
        m_CommandBuffers[m_CurrentFrame].pipelineBarrier2(dependencyInfo);
    }

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->m_IsFramebufferResized = true;
    }

    GLFWwindow* m_Window = nullptr;

    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    
    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
    vk::raii::Device m_Device = nullptr;
    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;
    uint32_t m_GraphicsQueueFamilyIndex;

    vk::raii::SurfaceKHR m_Surface = nullptr;
    vk::raii::SwapchainKHR m_SwapChain = nullptr;
    std::vector<vk::Image> m_SwapChainImages;
    std::vector<vk::raii::ImageView> m_SwapChainImageViews;
    vk::SurfaceFormatKHR m_SwapChainSurfaceFormat;
    vk::Extent2D m_SwapChainExtent;

    vk::raii::Pipeline m_GraphicsPipeline = nullptr;
    vk::raii::PipelineLayout m_PipelineLayout = nullptr;

    vk::raii::CommandPool m_CommandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> m_CommandBuffers;

    std::vector<vk::raii::Semaphore> m_ImageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_InFlightFences;

    uint32_t m_CurrentFrame = 0;
    bool m_IsFramebufferResized = false;
};

int main() {
    Application app;

    try {
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
