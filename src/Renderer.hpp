#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

#include <glm/glm.hpp>

#include "Swapchain.hpp"
#include "Buffer.hpp"

namespace Bjorn 
{
	// Note that for greater number of concurrent frames
	// the CPU might get ahead of the GPU causing latency
	// between frames
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	// TODO: comment
	constexpr uint32_t MAX_OBJECTS = 100;

	// Fwd declaration
	class Application;
	class Window;
	class Scene;
	class Mesh;

	struct GlobalUBO
	{
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct ObjectData
	{
		glm::mat4 model;
		// may be extended in the future
	};

	struct PushConstants
	{
		uint32_t objectIndex;
	};

	class Renderer 
	{
	public:
		Renderer(Application& app, const Window& window, const Scene& scene);
		~Renderer();

		void DrawFrame();
		void WaitIdle();

		void LoadMesh(Mesh& mesh);
		void CopyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, vk::DeviceSize size);

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

		VmaAllocator m_allocator;
		vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;
		std::array<std::unique_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT> m_globalUBOs;
		std::array<std::unique_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT> m_objectSSBOs;

		vk::raii::DescriptorPool m_descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> m_descriptorSets;

		vk::PushConstantRange m_pushConstantRange;

		vk::raii::PipelineLayout m_pipelineLayout = nullptr;
		vk::raii::Pipeline m_graphicsPipeline = nullptr;

		vk::raii::CommandPool m_commandPool = nullptr;
		std::vector<vk::raii::CommandBuffer> m_commandBuffers;

		std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
		std::vector<vk::raii::Fence> m_inFlightFences;

		void UpdateUniformBuffer();
		void UpdateOnFramebufferResized();

		void CreateInstance();
		void CreateSurface(); // May be moved to swapchain class
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateMemoryAllocator();
		void CreateSwapchain();
		void CreateDescriptorSetLayout();
		void CreatePushConstant();
		void CreateGraphicsPipeline();
		void CreateCommandPool();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
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
	};
}