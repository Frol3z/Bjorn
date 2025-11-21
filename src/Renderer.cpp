#include "Renderer.hpp"

#include "Application.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "GBuffer.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include <iostream>
#include <fstream>
#include <filesystem>

namespace Felina 
{
    // Validation layers
    #ifdef NDEBUG
        constexpr bool enableValidationLayers = false;
    #else
        constexpr bool enableValidationLayers = true;
    #endif

    const std::vector validationLayers =
    {
        "VK_LAYER_KHRONOS_validation",  // Khronos validation layers
        "VK_LAYER_LUNARG_monitor"       // FPS monitoring layer
    };

	Renderer::Renderer(Application& app, const Window& window, const Scene& scene)
        : m_app(app), m_window(window), m_scene(scene)
    {
        // Vulkan backend initialization
        CreateInstance();
        CreateSurface();
        CreateDevice();
        CreateSwapchain();
        CreateDescriptorPool();
        CreateGBuffer();
        CreateDescriptorSetLayout();
        CreatePushConstant();
        CreateForwardPipeline();
        CreateDeferredPipeline();
        CreateCommandPool();
        CreateCommandBuffer();
        CreateUniformBuffers();
        CreateDescriptorSets();
        CreateSyncObjects();
    }

    Renderer::~Renderer()
    {
        // Destroying buffers explicitly before freeing up memory
        for (auto& buffer : m_globalUBOs) {
            buffer.reset();
        }
        for (auto& buffer : m_objectSSBOs) {
            buffer.reset();
        }
    }

    void Renderer::DrawFrame()
    {
        // CPU will wait until the GPU finishes rendering the previous frame (corresponding to the same swapchain image index)
        while (vk::Result::eTimeout == m_device->GetDevice().waitForFences(*m_inFlightFences[m_currentFrame], vk::True, UINT64_MAX));

        // Wait for the presentation to release the semaphore
        m_device->GetPresentQueue().waitIdle();

        // Check if window has been resized/minimize before trying to acquire next image
        if (m_app.isFramebufferResized.load()) {
            UpdateOnFramebufferResized();
            return;
        }

        // Acquire next image index and signal the semaphore when done
        auto [result, imageIndex] = m_swapchain->AcquireNextImage(*m_imageAvailableSemaphores[m_currentFrame]);

        // Check if the surface is still compatible with the swapchain or if it was resized/minimized
        // !!! Checking m_IsFramebufferResized guarantees prevents presentation to an invalid surface
        if (result == vk::Result::eErrorOutOfDateKHR || m_app.isFramebufferResized.load()) {
            UpdateOnFramebufferResized();
            return;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        UpdateFrameData();

        // Record command buffer and reset draw fence
        m_device->GetDevice().resetFences(*m_inFlightFences[m_currentFrame]);
        RecordForwardCommandBuffer(imageIndex);

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
        m_device->GetGraphicsQueue().submit(submitInfo, *m_inFlightFences[m_currentFrame]);

        // Present to the screen
        const vk::PresentInfoKHR presentInfoKHR{
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &*m_renderFinishedSemaphores[m_currentFrame],
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain->GetHandle(),
            .pImageIndices = &imageIndex
        };
        result = m_device->GetPresentQueue().presentKHR(presentInfoKHR);

        // Check again if presentation fails because the surface is now incompatible
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_app.isFramebufferResized.load()) {
            UpdateOnFramebufferResized();
            return;
        }
        else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::WaitIdle()
    {
        m_device->GetDevice().waitIdle();
    }

    void Renderer::LoadMesh(Mesh& mesh)
    {
        mesh.Load(*m_device);
    }

    ImGui_ImplVulkan_InitInfo Renderer::GetImGuiInitInfo()
    {
        vk::PipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &m_swapchain->GetSurfaceFormat().format;
        pipelineRenderingCreateInfo.depthAttachmentFormat = vk::Format::eUndefined;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;

        // Create shader module
        auto shaderPath = std::filesystem::current_path() / "./shaders/imgui_custom.vert.spv";
        m_imGuiCustomVertShaderCode = ReadFile(shaderPath.string());
        m_imGuiCustomVertShaderModuleCreateInfo = {
            .codeSize = m_imGuiCustomVertShaderCode.size() * sizeof(char),
            .pCode = reinterpret_cast<const uint32_t*>(m_imGuiCustomVertShaderCode.data())
        };

        ImGui_ImplVulkan_InitInfo vkInitInfo = {};
        vkInitInfo.ApiVersion = vk::ApiVersion14;
        vkInitInfo.Instance = *m_instance;
        vkInitInfo.PhysicalDevice = *m_device->GetPhysicalDevice();
        vkInitInfo.Device = *m_device->GetDevice();
        vkInitInfo.QueueFamily = m_device->GetGraphicsQueueFamilyIndex();
        vkInitInfo.Queue = *m_device->GetGraphicsQueue();
        vkInitInfo.DescriptorPool = VK_NULL_HANDLE;
        vkInitInfo.DescriptorPoolSize = 1000; // ImGui backend will allocate the descriptor pool
        vkInitInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
        vkInitInfo.ImageCount = static_cast<uint32_t>(m_swapchain->GetImages().size());
        vkInitInfo.PipelineInfoMain.RenderPass = VK_NULL_HANDLE;
        vkInitInfo.PipelineInfoMain.Subpass = 0;
        vkInitInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        vkInitInfo.PipelineInfoMain.PipelineRenderingCreateInfo = *&pipelineRenderingCreateInfo;
        vkInitInfo.UseDynamicRendering = true;
        vkInitInfo.CustomShaderVertCreateInfo = *&m_imGuiCustomVertShaderModuleCreateInfo;
        vkInitInfo.CustomShaderFragCreateInfo = {};
        return vkInitInfo;
    }

    void Renderer::DrawImGuiFrame(ImDrawData* drawData)
    {
        ImGui_ImplVulkan_RenderDrawData(drawData, *m_commandBuffers[m_currentFrame]);
    }

    void Renderer::UpdateFrameData()
    {   
        // Update the global uniform buffer
        GlobalUBO globalUBO{};
        globalUBO.view = m_scene.GetCamera().GetViewMatrix();
        globalUBO.proj = m_scene.GetCamera().GetProjectionMatrix();
        m_globalUBOs[m_currentFrame]->LoadData(&globalUBO, sizeof(globalUBO));

        // Fill the object data storage buffer
        std::vector<Object*> objects = m_scene.GetObjects();
        std::vector<ObjectData> objectDatas;
        for (Object* obj : objects)
        {
            ObjectData data{};
            data.model = obj->GetModelMatrix();
            objectDatas.push_back(data);
        }
        m_objectSSBOs[m_currentFrame]->LoadData(objectDatas.data(), objectDatas.size() * sizeof(ObjectData));
    }

    void Renderer::UpdateOnFramebufferResized()
    {
        m_app.isFramebufferResized.store(false);
        m_swapchain->Recreate();

        for (size_t i = 0; i < m_gBuffers.size(); i++)
        {
            m_gBuffers[i]->Recreate(*m_device, m_swapchain->GetExtent(), m_descriptorPool);
        }
    }

	void Renderer::CreateInstance() 
    {
        // Optional appInfo struct
        const vk::ApplicationInfo appInfo
        {
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
            for (const auto& layer : layerProperties) 
                std::cout << '\t' << layer.layerName << std::endl;
            std::cout << "Available instance extensions:" << std::endl;
            for (const auto& extension : extensionProperties)
                std::cout << '\t' << extension.extensionName << std::endl;
        #endif

        // Get the required instance LAYERS
        std::vector<char const*> requiredLayers;
        // Validation layers (optional)
        if (enableValidationLayers)
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        
        // NOTE: add here additional LAYERS if needed (append to requiredLayers)
        
        // Check if the required LAYERS are supported by the Vulkan implementation
        if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
            return std::ranges::none_of(layerProperties,
                [requiredLayer](auto const& layerProperty)
                { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
            }))
        {
            throw std::runtime_error("One or more required layers are not supported!");
        }

        // Get the required instance EXTENSIONS from GLFW (e.g. VK_KHR_surface, VK_KHR_win32_surface)
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        // Check if the required GLFW extensions are supported by the Vulkan implementation
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            if (std::ranges::none_of(extensionProperties,
                [glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
                { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
            {
                throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
            }
        }

        // NOTE: add here additional EXTENSIONS if needed

        // Vulkan instance creation
        vk::InstanceCreateInfo createInfo
        {
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames = requiredLayers.data(),
            .enabledExtensionCount = glfwExtensionCount,
            .ppEnabledExtensionNames = glfwExtensions
        };

        try
        {
            m_instance = vk::raii::Instance(m_context, createInfo);
        }
        catch (const vk::SystemError& err)
        {
            std::cerr << "Vulkan error: " << err.what() << std::endl;
        }
    }

    void Renderer::CreateSurface()
    {
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(*m_instance, m_window.GetHandle(), nullptr, &_surface) != 0)
            throw std::runtime_error("Failed to create window surface!");
        m_surface = vk::raii::SurfaceKHR(m_instance, _surface);
    }

    void Renderer::CreateDevice()
    {
        m_device = std::make_unique<Device>(m_instance, m_surface);
    }

    void Renderer::CreateSwapchain()
    {
        m_swapchain = std::make_unique<Swapchain>(*m_device, m_window, m_surface);
    }

    void Renderer::CreateGBuffer()
    {
        for (size_t i = 0; i < m_gBuffers.size(); i++)
        {
            m_gBuffers[i] = std::make_unique<GBuffer>(
                *m_device,
                m_swapchain->GetExtent(),
                m_descriptorPool
            );
        }
    }

    void Renderer::CreateDescriptorSetLayout()
    {
        std::array<vk::DescriptorSetLayoutBinding, 2> bindings{};
        
        // Binding 0 -> global UBO
        bindings[0] = vk::DescriptorSetLayoutBinding {
            0, vk::DescriptorType::eUniformBuffer, 1,
            vk::ShaderStageFlagBits::eVertex, nullptr
        };

        // Binding 1 -> object SSBO
        bindings[1] = vk::DescriptorSetLayoutBinding{
            1, vk::DescriptorType::eStorageBuffer, 1,
            vk::ShaderStageFlagBits::eVertex, nullptr
        };

        // Create descriptor set layout
        vk::DescriptorSetLayoutCreateInfo layoutInfo{
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };
        m_descriptorSetLayout = vk::raii::DescriptorSetLayout(m_device->GetDevice(), layoutInfo);
    }

    void Renderer::CreatePushConstant()
    {
        m_pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
        m_pushConstantRange.offset = 0;
        m_pushConstantRange.size = sizeof(PushConstants);
    }

    void Renderer::CreateForwardPipeline()
    {
        // Create shader module
        auto vertShaderPath = std::filesystem::current_path() / "./shaders/forward.vert.spv";
        auto fragShaderPath = std::filesystem::current_path() / "./shaders/forward.frag.spv";
        vk::raii::ShaderModule vertShaderModule = CreateShaderModule(ReadFile(vertShaderPath.string()));
        vk::raii::ShaderModule fragShaderModule = CreateShaderModule(ReadFile(fragShaderPath.string()));

        // ShaderStageCreateInfo
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertShaderModule,
            .pName = "vertMain"
        };
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragShaderModule,
            .pName = "fragMain"
        };
        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // VertexInput
        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = attributeDescriptions.size(),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        };

        // InputAssembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly { .topology = vk::PrimitiveTopology::eTriangleList };

        // Dynamic viewport and scissors placeholders (will be set through the command buffer)
        std::vector dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo dynamicState{
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };
        // We just need to specify how many there are at pipeline creation time
        vk::PipelineViewportStateCreateInfo viewportState { .viewportCount = 1, .scissorCount = 1 };

        // Rasterizer
        vk::PipelineRasterizationStateCreateInfo rasterizer {
            .depthClampEnable = vk::False, // Clamp out of bound depth values to the near or far plane
            .rasterizerDiscardEnable = vk::False, // Disable rasterization (no output)
            .polygonMode = vk::PolygonMode::eFill, // For wireframe mode a GPU feature must be enabled
            .cullMode = vk::CullModeFlagBits::eBack, // Self explanatory
            .frontFace = vk::FrontFace::eCounterClockwise,
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

        // Pipeline layout (descriptors layout and push constants)
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &*m_descriptorSetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &m_pushConstantRange
        };
        m_fwdPipelineLayout = vk::raii::PipelineLayout(m_device->GetDevice(), pipelineLayoutInfo);

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
            .layout = m_fwdPipelineLayout,
            .renderPass = nullptr // Because we are using dynamic rendering
        };

        // FINALLY!
        m_fwdPipeline = vk::raii::Pipeline(m_device->GetDevice(), nullptr, pipelineInfo);
    }

    void Renderer::CreateDeferredPipeline()
    {
        // TODO: refactor
        // - abstract pipeline object
        // - improve shader handling

        auto& gBuffer = m_gBuffers[0];

        // ---- GEOMETRY PASS ----

        // Create shader module
        // TODO: use deferred shader
        auto vertShaderPath = std::filesystem::current_path() / "./shaders/forward.vert.spv";
        auto fragShaderPath = std::filesystem::current_path() / "./shaders/forward.frag.spv";
        vk::raii::ShaderModule vertShaderModule = CreateShaderModule(ReadFile(vertShaderPath.string()));
        vk::raii::ShaderModule fragShaderModule = CreateShaderModule(ReadFile(fragShaderPath.string()));

        // ShaderStageCreateInfo
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertShaderModule,
            .pName = "vertMain"
        };
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragShaderModule,
            .pName = "fragMain"
        };
        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // VertexInput
        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = attributeDescriptions.size(),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        };

        // InputAssembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = vk::PrimitiveTopology::eTriangleList };

        // Dynamic viewport and scissors placeholders (will be set through the command buffer)
        std::vector dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
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
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = vk::False,
            .depthBiasSlopeFactor = 1.0f,
            .lineWidth = 1.0f
        };

        // MSAA (currently disabled because it requires enabling a GPU feature)
        vk::PipelineMultisampleStateCreateInfo multisampling{
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = vk::False
        };

        // Depth (and stencil) attachment
        vk::PipelineDepthStencilStateCreateInfo depthStencil{
            .depthTestEnable = vk::True,  // enable depth testing
            .depthWriteEnable = vk::True, // enable writing to the depth attachment
            .depthCompareOp = vk::CompareOp::eLess, // if fragment < depth then the test is passed (+Z forward)
            .depthBoundsTestEnable = vk::False,
            .stencilTestEnable = vk::False // stencil test disabled for now
        };

        // Color blending (disabled)
        // NOTE: NOT SPECIFIED for the depth attachments
        std::vector <vk::PipelineColorBlendAttachmentState> colorBlendAttachments(gBuffer->GetAttachmentsCount() - 1);
        for (auto& attachment : colorBlendAttachments)
        {
            attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
            attachment.blendEnable = vk::False;
        }

        // Blending with bitwise operations
        vk::PipelineColorBlendStateCreateInfo colorBlending{
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size()),
            .pAttachments = colorBlendAttachments.data()
        };

        // Pipeline layout (descriptors layout and push constants)
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &*m_descriptorSetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &m_pushConstantRange
        };
        m_defGeometryPipelineLayout = vk::raii::PipelineLayout(m_device->GetDevice(), pipelineLayoutInfo);

        // PipelineRenderingCreateInfo
        // It specifies the formats of the attachment used with dynamic rendering
        auto colorAttachmentFormats = gBuffer->GetColorAttachmentFormats();
        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
            .colorAttachmentCount = static_cast<uint32_t>(colorBlendAttachments.size()),
            .pColorAttachmentFormats = colorAttachmentFormats.data(),
            .depthAttachmentFormat = gBuffer->GetDepthFormat()
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
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = m_defGeometryPipelineLayout,
            .renderPass = nullptr // Because we are using dynamic rendering
        };
        m_defGeometryPipeline = vk::raii::Pipeline(m_device->GetDevice(), nullptr, pipelineInfo);

        // ---- LIGHTING PASS ----

        // Create shader module
        auto lightingVertShaderPath = std::filesystem::current_path() / "./shaders/forward.vert.spv";
        auto lightingFragShaderPath = std::filesystem::current_path() / "./shaders/forward.frag.spv";
        vk::raii::ShaderModule lightingVertShaderModule = CreateShaderModule(ReadFile(lightingVertShaderPath.string()));
        vk::raii::ShaderModule lightingFragShaderModule = CreateShaderModule(ReadFile(lightingFragShaderPath.string()));

        // ShaderStageCreateInfo
        vk::PipelineShaderStageCreateInfo lightingVertShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = lightingVertShaderModule,
            .pName = "vertMain"
        };
        vk::PipelineShaderStageCreateInfo lightingFragShaderStageInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = lightingFragShaderModule,
            .pName = "fragMain"
        };
        vk::PipelineShaderStageCreateInfo lightingShaderStages[] = { lightingVertShaderStageInfo, lightingFragShaderStageInfo };

        // VertexInput (empty)
        // NOTE: a full screen quad will we drawn at draw time
        vk::PipelineVertexInputStateCreateInfo lightingVertexInputInfo{};

        // Color blending (disabled)
        vk::PipelineColorBlendAttachmentState lightingColorBlendAttachment{
            .blendEnable = vk::False,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                              vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
        };

        // Blending with bitwise operations
        vk::PipelineColorBlendStateCreateInfo lightingColorBlending{
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &lightingColorBlendAttachment
        };

        // Pipeline layout (descriptors layout and push constants)
        vk::PipelineLayoutCreateInfo lightingPipelineLayoutInfo{
            .setLayoutCount = 1,
            .pSetLayouts = &*gBuffer->GetDescriptorSetLayout(),
        };
        m_defLightingPipelineLayout = vk::raii::PipelineLayout(m_device->GetDevice(), lightingPipelineLayoutInfo);

        // PipelineRenderingCreateInfo
        // It specifies the formats of the attachment used with dynamic rendering
        vk::PipelineRenderingCreateInfo lightingPipelineRenderingCreateInfo{
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &m_swapchain->GetSurfaceFormat().format
        };

        // GraphicsPipelineCreateInfo
        vk::GraphicsPipelineCreateInfo lightingPipelineInfo{
            .pNext = &lightingPipelineRenderingCreateInfo,
            .stageCount = 2,
            .pStages = lightingShaderStages,
            .pVertexInputState = &lightingVertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &lightingColorBlending,
            .pDynamicState = &dynamicState,
            .layout = m_defLightingPipelineLayout,
            .renderPass = nullptr // Because we are using dynamic rendering
        };
        m_defLightingPipeline = vk::raii::Pipeline(m_device->GetDevice(), nullptr, pipelineInfo);
    }

    vk::raii::ShaderModule Renderer::CreateShaderModule(const std::vector<char>& code) const
    {
        vk::ShaderModuleCreateInfo createInfo{
            .codeSize = code.size() * sizeof(char),
            .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };
        vk::raii::ShaderModule shaderModule(m_device->GetDevice(), createInfo);
        return shaderModule;
    }

    std::vector<char> Renderer::ReadFile(const std::string& filename) 
    {
        // Read starting from the end to determine the size of the file
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
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
            .queueFamilyIndex = m_device->GetGraphicsQueueFamilyIndex()
        };
        m_commandPool = vk::raii::CommandPool(m_device->GetDevice(), poolInfo);
    }

    void Renderer::CreateCommandBuffer()
    {
        // CommandBufferAllocateInfo
        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool = m_commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = MAX_FRAMES_IN_FLIGHT
        };
        m_commandBuffers = vk::raii::CommandBuffers(m_device->GetDevice(), allocInfo);
    }

    void Renderer::CreateUniformBuffers()
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            // Global uniform buffer creation
            vk::BufferCreateInfo uboInfo{};
            uboInfo.size = sizeof(GlobalUBO);
            uboInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
            VmaAllocationCreateInfo uboAllocInfo{};
            uboAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            m_globalUBOs[i] = std::make_unique<Buffer>(m_device->GetAllocator(), uboInfo, uboAllocInfo, true);

            // Object data storage buffer creation
            vk::BufferCreateInfo ssboInfo{};
            ssboInfo.size = sizeof(ObjectData) * MAX_OBJECTS;
            ssboInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;
            VmaAllocationCreateInfo ssboAllocInfo{};
            ssboAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            m_objectSSBOs[i] = std::make_unique<Buffer>(m_device->GetAllocator(), ssboInfo, ssboAllocInfo, true);
        }
    }

    void Renderer::CreateDescriptorPool()
    {
        // TODO: remove hardcoded max number of combined sampler
        uint32_t attachmentsCount = 4 * MAX_FRAMES_IN_FLIGHT;
        std::array<vk::DescriptorPoolSize, 3> poolSizes {
            vk::DescriptorPoolSize { .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = MAX_FRAMES_IN_FLIGHT },
            vk::DescriptorPoolSize { .type = vk::DescriptorType::eStorageBuffer, .descriptorCount = MAX_FRAMES_IN_FLIGHT },
            vk::DescriptorPoolSize { 
                .type = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = attachmentsCount
            }
        };

        // NOTE:
        // Current sets:
        // - global UBO and SSBO set (see below)
        // - GBuffer (see GBuffer class)
        // Multiplied by MAX_FRAMES_IN_FLIGHT

        vk::DescriptorPoolCreateInfo poolInfo{
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
            .maxSets = MAX_FRAMES_IN_FLIGHT * 2, // TODO: remove hardcoded number of sets
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };
        m_descriptorPool = vk::raii::DescriptorPool(m_device->GetDevice(), poolInfo);
    }

    void Renderer::CreateDescriptorSets()
    {
        // Descriptor sets allocation
        std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
            .pSetLayouts = layouts.data()
        };
        m_descriptorSets = m_device->GetDevice().allocateDescriptorSets(allocInfo);

        // Bind the actual buffers to the descriptor sets
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vk::DescriptorBufferInfo globalUBOInfo{
                .buffer = m_globalUBOs[i]->GetHandle(),
                .offset = 0,
                .range = sizeof(GlobalUBO)
            };
            vk::DescriptorBufferInfo objectSSBOInfo{
                .buffer = m_objectSSBOs[i]->GetHandle(),
                .offset = 0,
                .range = sizeof(ObjectData) * MAX_OBJECTS
            };

            constexpr size_t bindingsCount = 2;
            std::array<vk::WriteDescriptorSet, bindingsCount> descriptorWrites{};

            // Binding 0 -> Global UBO
            descriptorWrites[0] = vk::WriteDescriptorSet {
                .dstSet = m_descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &globalUBOInfo
            };

            // Binding 1 -> Object SSBO
            descriptorWrites[1] = vk::WriteDescriptorSet {
                .dstSet = m_descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eStorageBuffer,
                .pBufferInfo = &objectSSBOInfo
            };

            m_device->GetDevice().updateDescriptorSets(descriptorWrites, {});
        }
    }

    void Renderer::CreateSyncObjects()
    {
        m_imageAvailableSemaphores.clear();
        m_renderFinishedSemaphores.clear();
        m_inFlightFences.clear();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_imageAvailableSemaphores.emplace_back(m_device->GetDevice(), vk::SemaphoreCreateInfo());
            m_renderFinishedSemaphores.emplace_back(m_device->GetDevice(), vk::SemaphoreCreateInfo());
            m_inFlightFences.emplace_back(m_device->GetDevice(), vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
        }
    }

    void Renderer::RecordForwardCommandBuffer(uint32_t imageIndex)
    {
        // Reset command buffer
        m_commandBuffers[m_currentFrame].begin({});

        // Transition image layout to color attachment
        TransitionImageLayout(
            m_swapchain->GetImages()[imageIndex],
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
        m_commandBuffers[m_currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, m_fwdPipeline);

        // Set viewport and scissor size (dynamic rendering)
        m_commandBuffers[m_currentFrame].setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchainExtent.width), static_cast<float>(swapchainExtent.height), 0.0f, 1.0f));
        m_commandBuffers[m_currentFrame].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchainExtent));

        // Bind descriptor sets
        m_commandBuffers[m_currentFrame].bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, m_fwdPipelineLayout, 0,
            *m_descriptorSets[m_currentFrame], nullptr
        );
        
        // Draw all the objects
        std::vector<Object*> objects = m_scene.GetObjects();
        for (uint32_t i = 0; i < objects.size(); i++)
        {
            Object* obj = objects[i];

            // Update object index
            PushConstants pc{ i };
            m_commandBuffers[m_currentFrame].pushConstants(
                *m_fwdPipelineLayout,
                vk::ShaderStageFlagBits::eVertex,
                0,
                vk::ArrayProxy<const PushConstants>(1, &pc)
            );

            // Bind vertex and index buffer
            auto& mesh = obj->GetMesh();
            m_commandBuffers[m_currentFrame].bindVertexBuffers(0, mesh.GetVertexBuffer().GetHandle(), { 0 });
            m_commandBuffers[m_currentFrame].bindIndexBuffer(mesh.GetIndexBuffer().GetHandle(), 0, mesh.GetIndexType());

            // Draw call
            m_commandBuffers[m_currentFrame].drawIndexed(mesh.GetIndexBufferSize(), 1, 0, 0, 0);
        }

        // Draw Dear ImGui
        DrawImGuiFrame(ImGui::GetDrawData());

        // Finish up rendering
        m_commandBuffers[m_currentFrame].endRendering();

        // After rendering, transition the swapchain image to PRESENT_SRC
        TransitionImageLayout(
            m_swapchain->GetImages()[imageIndex],
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

    void Renderer::RecordDeferredCommandBuffer(uint32_t imageIndex)
    {
        auto& cmdBuf = m_commandBuffers[m_currentFrame];
        auto& gBuffer = m_gBuffers[m_currentFrame];
        vk::Extent2D swapchainExtent = m_swapchain->GetExtent();

        cmdBuf.begin({});
        
        // ---- Geometry pass ----

        // Transition G-buffer attachments to color attachment layout
        const GBuffer::Attachment* depthAttachment = nullptr;
        for (auto& attachment : gBuffer->GetAttachments())
        {
            if (attachment.type == GBuffer::AttachmentType::Depth)
            {
                depthAttachment = &attachment;
                continue;
            }
                
            TransitionImageLayout(
                attachment.image->GetHandle(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
                {}, vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput
            );
        }

        if(depthAttachment == nullptr)
            throw std::runtime_error("[RENDERER] Couldn't retrieve the G-buffer depth attachment because there's no such attachment!");

        // Transition depth attachment as well
        TransitionImageLayout(
            depthAttachment->image->GetHandle(), vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
            {}, vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eEarlyFragmentTests
        );

        // Setup rendering info (dynamic rendering)
        // Color attachments rendering info
        std::vector<vk::RenderingAttachmentInfo> colorAttachmentInfos;
        for (auto& attachment : gBuffer->GetAttachments())
        {
            if (attachment.type == GBuffer::AttachmentType::Depth)
                continue;
            colorAttachmentInfos.push_back({
                .imageView = attachment.image->GetImageView(),
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp = vk::AttachmentLoadOp::eClear,
                .storeOp = vk::AttachmentStoreOp::eStore,
                .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)
            });
        }
        // Depth attachment rendering info
        vk::RenderingAttachmentInfo depthAttachmentInfo{
            .imageView = depthAttachment->image->GetImageView(),
            .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearDepthStencilValue{1.0f, 0} // {depth, stencil} -> 1.0f - far plane
        };
        vk::RenderingInfo lightingRenderingInfo = {
            .renderArea = {.offset = { 0, 0 }, .extent = swapchainExtent},
            .layerCount = 1,
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size()),
            .pColorAttachments = colorAttachmentInfos.data(),
            .pDepthAttachment = &depthAttachmentInfo
        };

        // Begin rendering
        cmdBuf.beginRendering(lightingRenderingInfo);
        
        // Bind the graphic pipeline (the attachment will be bound to the fragment shader output)
        m_commandBuffers[m_currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, m_defGeometryPipeline);

        // Set viewport and scissor size (dynamic rendering)
        m_commandBuffers[m_currentFrame].setViewport(
            0,
            vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchainExtent.width), static_cast<float>(swapchainExtent.height), 0.0f, 1.0f)
        );
        m_commandBuffers[m_currentFrame].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchainExtent));

        // Bind descriptor sets (global UBO, object SSBO)
        m_commandBuffers[m_currentFrame].bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, m_defGeometryPipelineLayout, 0,
            *m_descriptorSets[m_currentFrame], nullptr
        );

        // Draw all the objects
        std::vector<Object*> objects = m_scene.GetObjects();
        for (uint32_t i = 0; i < objects.size(); i++)
        {
            Object* obj = objects[i];

            // Update object index
            PushConstants pc{ i };
            m_commandBuffers[m_currentFrame].pushConstants(
                *m_defGeometryPipelineLayout,
                vk::ShaderStageFlagBits::eVertex,
                0,
                vk::ArrayProxy<const PushConstants>(1, &pc)
            );

            // Bind vertex and index buffer
            auto& mesh = obj->GetMesh();
            m_commandBuffers[m_currentFrame].bindVertexBuffers(0, mesh.GetVertexBuffer().GetHandle(), { 0 });
            m_commandBuffers[m_currentFrame].bindIndexBuffer(mesh.GetIndexBuffer().GetHandle(), 0, mesh.GetIndexType());

            // Draw call
            m_commandBuffers[m_currentFrame].drawIndexed(mesh.GetIndexBufferSize(), 1, 0, 0, 0);
        }

        // Draw Dear ImGui
        DrawImGuiFrame(ImGui::GetDrawData());

        cmdBuf.endRendering();
    
        // ---- Lighting pass ----

        // Transition G-buffer attachments to shader read layout for sampling
        for (auto& attachment : gBuffer->GetAttachments())
        {
            TransitionImageLayout(
                attachment.image->GetHandle(), 
                attachment.type == GBuffer::AttachmentType::Depth ? vk::ImageLayout::eDepthAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal, 
                vk::ImageLayout::eShaderReadOnlyOptimal,
                {}, 
                attachment.type == GBuffer::AttachmentType::Depth ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eColorAttachmentRead,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eFragmentShader
            );
        }

        // Transition swapchain image to color attachment layout
        TransitionImageLayout(
            m_swapchain->GetImages()[imageIndex],
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            {},
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );

        // Setup rendering info
        vk::RenderingAttachmentInfo finalAttachmentInfo{
            .imageView = m_swapchain->GetImageViews()[imageIndex],
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)
        };
        vk::RenderingInfo renderingInfo = {
            .renderArea = {.offset = { 0, 0 }, .extent = swapchainExtent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &finalAttachmentInfo
        };

        // Rendering (computing lighting)
        cmdBuf.beginRendering(renderingInfo);

        // Bind the graphic pipeline (the attachment will be bound to the fragment shader output)
        m_commandBuffers[m_currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, m_defLightingPipeline);

        // Set viewport and scissor size (dynamic rendering)
        m_commandBuffers[m_currentFrame].setViewport(
            0,
            vk::Viewport(0.0f, 0.0f, static_cast<float>(swapchainExtent.width), static_cast<float>(swapchainExtent.height), 0.0f, 1.0f)
        );
        m_commandBuffers[m_currentFrame].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapchainExtent));

        // Bind descriptor sets (global UBO, object SSBO)
        m_commandBuffers[m_currentFrame].bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, m_defGeometryPipelineLayout, 0,
            *gBuffer->GetDescriptorSet(), nullptr
        );

        // Draw a triangle that covers the screen (optimization of a quad)
        cmdBuf.draw(3, 1, 0, 0);

        cmdBuf.endRendering();

        // After rendering, transition the swapchain image to PRESENT_SRC
        TransitionImageLayout(
            m_swapchain->GetImages()[imageIndex],
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            {},
            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            vk::PipelineStageFlagBits2::eBottomOfPipe
        );

        cmdBuf.end();
    }

    void Renderer::TransitionImageLayout(
        vk::Image image,
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
            .image = image,
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
}