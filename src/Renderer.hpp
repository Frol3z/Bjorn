#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Bjorn {
	class Renderer {
	public:
		Renderer(const std::string& appName);

	private:
		vk::raii::Context m_context;
		vk::raii::Instance m_instance = nullptr;

		void CreateInstance();

		/* TODO:
		void CreateSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
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

		const std::string& m_appName;
	};
}