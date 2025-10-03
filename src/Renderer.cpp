#include "Renderer.hpp"
#include "Window.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

// Validation layers
#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif

const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

namespace Bjorn {
	Renderer::Renderer(const std::string& appName, const Window& window)
        : m_appName(appName), m_window(window)
    {
        CreateInstance();
        CreateSurface();
        SelectPhysicalDevice();
        CreateLogicalDevice();
    }

	void Renderer::CreateInstance() {
        // Optional appInfo struct
        const vk::ApplicationInfo appInfo{
            .pApplicationName = m_appName.c_str(),
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

    void Renderer::CreateSurface() {
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(*m_instance, m_window.GetHandle(), nullptr, &_surface) != 0) {
            throw std::runtime_error("Failed to create window surface!");
        }
        m_surface = vk::raii::SurfaceKHR(m_instance, _surface);
    }

    void Renderer::SelectPhysicalDevice() {
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

    void Renderer::CreateLogicalDevice() {
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
}