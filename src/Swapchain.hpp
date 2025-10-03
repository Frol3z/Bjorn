#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Bjorn 
{
	class Window;

	class Swapchain 
	{
	public:
		Swapchain(
			const vk::raii::PhysicalDevice& physicalDevice, 
			const vk::raii::Device& device,
			const Window& window,
			const vk::raii::SurfaceKHR& surface
		);

		//void RecreateSwapChain();
		//void CleanUpSwapChain();
		//void CreateImageViews();
	private:
		const vk::raii::PhysicalDevice& m_physicalDevice;
		const vk::raii::Device& m_device;
		const vk::raii::SurfaceKHR& m_surface;
		const Window& m_window;

		vk::raii::SwapchainKHR m_swapchain = nullptr;
		std::vector<vk::Image> m_swapchainImages;
		vk::SurfaceFormatKHR m_swapchainSurfaceFormat;
		vk::Extent2D m_swapchainExtent;
\
		void CreateSwapchain();
		
		vk::SurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
		vk::Extent2D ChooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
		vk::PresentModeKHR ChooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
	};
}