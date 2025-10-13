#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

#include "Swapchain.hpp"

namespace Bjorn 
{

	// Fwd declaration
	class Application;
	class Window;
	class Scene;

	class Renderer 
	{
	public:
		Renderer(Application& app, const Window& window, const Scene& scene);
		~Renderer();

		void DrawFrame();
		void WaitIdle();

	private:
		Application& m_app; // Need to be able to modify the framebufferResized boolean
		const Window& m_window;
		const Scene& m_scene;

		uint32_t m_currentFrame = 0;

		vk::raii::Context m_context;
		vk::raii::Instance m_instance = nullptr;
		vk::raii::SurfaceKHR m_surface = nullptr;

		vk::raii::PhysicalDevice m_physicalDevice = nullptr;
		vk::raii::Device m_device = nullptr;
		vk::raii::Queue m_graphicsQueue = nullptr;
		vk::raii::Queue m_presentQueue = nullptr;
		uint32_t m_graphicsQueueFamilyIndex;

		std::unique_ptr<Swapchain> m_swapchain = nullptr;

		vk::raii::Pipeline m_graphicsPipeline = nullptr;
		vk::raii::PipelineLayout m_pipelineLayout = nullptr;

		vk::raii::CommandPool m_commandPool = nullptr;
		std::vector<vk::raii::CommandBuffer> m_commandBuffers;

		std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
		std::vector<vk::raii::Fence> m_inFlightFences;

		VmaAllocator m_allocator;
		vk::raii::Buffer m_stagingBuffer = nullptr;
		VmaAllocation m_stagingBufferAllocation;

		vk::raii::Buffer m_vertexBuffer = nullptr;
		VmaAllocation m_vertexBufferAllocation;

		vk::raii::Buffer m_indexBuffer = nullptr;
		VmaAllocation m_indexBufferAllocation;

		void CreateInstance();
		void CreateSurface(); // May be moved to swapchain class
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateMemoryAllocator();
		void CreateSwapchain();
		void CreateGraphicsPipeline();
		void CreateCommandPool();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateCommandBuffer();
		void CreateSyncObjects();

		[[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code) const;
		static std::vector<char> ReadFile(const std::string& filename);

		void RecordCommandBuffer(uint32_t imageIndex);
		void TransitionImageLayout(
			uint32_t imageIndex,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 srcStageMask,
			vk::PipelineStageFlags2 dstStageMask
		);

		void CopyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
	};
}