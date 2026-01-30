#include "Swapchain.hpp"

#include "Window.hpp"
#include "Device.hpp"

#include "GLFW/glfw3.h"

namespace Felina 
{
	Swapchain::Swapchain(const Device& device, const Window& window, const vk::raii::SurfaceKHR& surface)
        : m_device(device), m_surface(surface), m_window(window)
	{
		Create();
        CreateImageViews();
	}

    void Swapchain::Recreate() 
    {
        // Handling minimization
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window.GetHandle(), &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window.GetHandle(), &width, &height);
            glfwWaitEvents(); // "Pausing" when minimized
        }

        m_device.GetDevice().waitIdle();

        CleanUp();
        Create();
        CreateImageViews();
    }

    vk::ResultValue<uint32_t> Swapchain::AcquireNextImage(const vk::Semaphore& s)
    {
        return m_swapchain.acquireNextImage(UINT64_MAX, s, nullptr);
    }

	void Swapchain::Create()
	{
        const auto& physicalDevice = m_device.GetPhysicalDevice();

        // Query for surface capabilities (min/max number of images, min/max width and height)
        auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
        // Query for surface format (pixel format, color space)
        std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(m_surface);
        // Query for available presentation modes
        std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(m_surface);

        // Choose suitable surface format and extent
        m_swapchainSurfaceFormat = ChooseSurfaceFormat(availableFormats);
        m_swapchainExtent = ChooseExtent(surfaceCapabilities);

        // Choose how many images to have in the swapchain
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        minImageCount = (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount)
            ? surfaceCapabilities.maxImageCount
            : minImageCount;

        // SwapChainCreateInfo
        // If rendering to a separate image (e.g. to apply post-processing effects) and then
        // presenting to the screen imageUsage should be set to VK_IMAGE_USAGE_TRANSFER_DST_BIT
        vk::SwapchainCreateInfoKHR swapchainCreateInfo
        {
            .flags = vk::SwapchainCreateFlagsKHR(),
            .surface = m_surface,
            .minImageCount = minImageCount,
            .imageFormat = m_swapchainSurfaceFormat.format,
            .imageColorSpace = m_swapchainSurfaceFormat.colorSpace,
            .imageExtent = m_swapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive, // NOTE: assuming graphics and presentation queue family is the same
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = ChoosePresentMode(availablePresentModes),
            .clipped = true,
            .oldSwapchain = nullptr
        };

        // Create swap chain
        m_swapchain = vk::raii::SwapchainKHR(m_device.GetDevice(), swapchainCreateInfo);
        m_swapchainImages = m_swapchain.getImages();
	}

    void Swapchain::CleanUp()
    {
        m_swapchainImageViews.clear();
        m_swapchain = nullptr;
    }

    void Swapchain::CreateImageViews()
    {
        m_swapchainImageViews.clear();

        // ImageViewCreateInfo
        vk::ImageViewCreateInfo imageViewCreateInfo
        {
            .viewType = vk::ImageViewType::e2D,
            .format = m_swapchainSurfaceFormat.format,
            .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
        };

        // Create image views for each image
        for (auto image : m_swapchainImages)
        {
            imageViewCreateInfo.image = image;
            m_swapchainImageViews.emplace_back(m_device.GetDevice(), imageViewCreateInfo);
        }
    }

    vk::SurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                return availableFormat;
        }
        return availableFormats[0];
    }

    vk::Extent2D Swapchain::ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        int width, height;
        glfwGetFramebufferSize(m_window.GetHandle(), &width, &height);
        
        return {
            std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }

    vk::PresentModeKHR Swapchain::ChoosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
    {
        // NOTE: it should be using mailbox but I want vsync rn
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eFifo)
                return availablePresentMode;
        }
        // The only mode guaranteed to be available (may result in visible tearing)
        return vk::PresentModeKHR::eFifo;
    }
}