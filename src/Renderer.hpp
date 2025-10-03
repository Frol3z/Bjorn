#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Bjorn {

	// Fwd declaration
	class Window;

	class Renderer {
	public:
		Renderer(const std::string& appName, const Window& window);

	private:
		const std::string& m_appName;

		const Window& m_window;

		vk::raii::Context m_context;
		vk::raii::Instance m_instance = nullptr;
		vk::raii::SurfaceKHR m_surface = nullptr;

		vk::raii::PhysicalDevice m_physicalDevice = nullptr;
		vk::raii::Device m_device = nullptr;
		vk::raii::Queue m_graphicsQueue = nullptr;
		vk::raii::Queue m_presentQueue = nullptr;
		uint32_t m_graphicsQueueFamilyIndex;


		void CreateInstance();
		void CreateSurface(); // May be moved to swapchain class
		void SelectPhysicalDevice();
		void CreateLogicalDevice();

		/* TODO:
		void CreateSwapChain();
		void RecreateSwapChain();
		void CleanUpSwapChain();
		void CreateImageViews();
		void CreateGraphicsPipeline();
		void CreateCommandPool();
		void CreateCommandBuffer();
		void CreateSyncObjects();

		vk::SurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
		vk::PresentModeKHR ChooseSwapChainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
		vk::Extent2D ChooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

		void RecordCommandBuffer(uint32_t imageIndex);

		static std::vector<char> ReadFile(const std::string& filename);
		[[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code) const;
		void TransitionImageLayout(
			uint32_t imageIndex,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 srcStageMask,
			vk::PipelineStageFlags2 dstStageMask
		);
		*/
	};
}