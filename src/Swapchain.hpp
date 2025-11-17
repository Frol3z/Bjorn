#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Felina
{
	// Fwd declarations
	class Window;
	class Device;

	class Swapchain 
	{
		public:
			Swapchain(const Device& device, const Window& window, const vk::raii::SurfaceKHR& surface);

			void RecreateSwapchain();
			std::pair<vk::Result, uint32_t> AcquireNextImage(const vk::Semaphore& s);

			const vk::SwapchainKHR& GetHandle() const { return *m_swapchain; }
			const vk::SurfaceFormatKHR& GetSurfaceFormat() const { return m_swapchainSurfaceFormat; }
			const vk::Extent2D& GetExtent() const { return m_swapchainExtent; }
			const std::vector<vk::Image>& GetImages() const { return m_swapchainImages; }
			const std::vector<vk::raii::ImageView>& GetImageViews() const { return m_swapchainImageViews; }

		private:
			void CreateSwapchain();
			void CleanUpSwapchain();
			void CreateImageViews();
		
			vk::SurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
			vk::Extent2D ChooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
			vk::PresentModeKHR ChooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

			const Device& m_device;
			const vk::raii::SurfaceKHR& m_surface;
			const Window& m_window;

			vk::raii::SwapchainKHR m_swapchain = nullptr;
			std::vector<vk::Image> m_swapchainImages;
			std::vector<vk::raii::ImageView> m_swapchainImageViews;

			vk::SurfaceFormatKHR m_swapchainSurfaceFormat;
			vk::Extent2D m_swapchainExtent;
	};
}