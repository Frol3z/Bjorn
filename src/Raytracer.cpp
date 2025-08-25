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
        }
    }

    void CleanUp() {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void InitWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Application", nullptr, nullptr);
    }

    void InitVulkan() {
        CreateInstance();
        CreateSurface();
        SelectPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateImageViews();
        CreateGraphicsPipeline();
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
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
            featureChain = {
                {},
                {.dynamicRendering = true},
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

    GLFWwindow* m_Window = nullptr;

    vk::raii::Context m_Context;
    vk::raii::Instance m_Instance = nullptr;
    
    vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
    vk::raii::Device m_Device = nullptr;
    vk::raii::Queue m_GraphicsQueue = nullptr;
    vk::raii::Queue m_PresentQueue = nullptr;

    vk::raii::SurfaceKHR m_Surface = nullptr;

    vk::raii::SwapchainKHR m_SwapChain = nullptr;
    std::vector<vk::Image> m_SwapChainImages;
    std::vector<vk::raii::ImageView> m_SwapChainImageViews;
    vk::SurfaceFormatKHR m_SwapChainSurfaceFormat;
    vk::Extent2D m_SwapChainExtent;
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
