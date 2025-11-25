#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

#include <glm/glm.hpp>

struct ImGui_ImplVulkan_InitInfo;
struct ImDrawData;

namespace Felina
{
	// NOTE: 
	// for a greater number of concurrent frames
	// the CPU might get ahead of the GPU causing latency
	// between frames
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	// Max number of drawable objects
	constexpr uint32_t MAX_OBJECTS = 100;

	// Fwd declaration
	class Application;
	class Device;
	class Swapchain;
	class GBuffer;
	class Buffer;
	class Window;
	class Scene;
	class Mesh;


	class Renderer
	{
		public:
			// TODO: allow swapping mode on the fly/choosing at startup
			enum RenderMode { FORWARD = 0, DEFERRED };
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

		public:
			Renderer(Application& app, const Window& window, const Scene& scene);
			~Renderer();

			void DrawFrame();
			void WaitIdle();
			void LoadMesh(Mesh& mesh);

			// Dear ImGui
			ImGui_ImplVulkan_InitInfo GetImGuiInitInfo();
			void DrawImGuiFrame(ImDrawData* drawData);

		private:
			void UpdateFrameData();
			void UpdateOnFramebufferResized();

			void CreateInstance();
			void CreateSurface();
			void CreateDevice();
			void CreateSwapchain();
			void CreateGBuffer();
			void CreateDescriptorSetLayout();
			void CreatePushConstant();
			void CreateForwardPipeline();
			void CreateDeferredPipeline();
			void CreateCommandPool();
			void CreateCommandBuffer();
			void CreateUniformBuffers();
			void CreateDescriptorPool();
			void CreateDescriptorSets();
			void CreateSyncObjects();

			void RecordForwardCommandBuffer(uint32_t imageIndex);
			void RecordDeferredCommandBuffer(uint32_t imageIndex);
			void TransitionImageLayout(
				vk::Image image,
				vk::Format imageFormat,
				vk::ImageLayout oldLayout,
				vk::ImageLayout newLayout,
				vk::AccessFlags2 srcAccessMask,
				vk::AccessFlags2 dstAccessMask,
				vk::PipelineStageFlags2 srcStageMask,
				vk::PipelineStageFlags2 dstStageMask
			);

			Application& m_app; // Need to be able to modify the framebufferResized boolean
			const Window& m_window;
			const Scene& m_scene;

			uint32_t m_currentFrame = 0;

			// Dear ImGui custom vertex shader (temporary fix for colors issue)
			std::vector<char> m_imGuiCustomVertShaderCode;
			vk::ShaderModuleCreateInfo m_imGuiCustomVertShaderModuleCreateInfo;

			vk::raii::Context m_context; // Used as a dynamic loader
			vk::raii::Instance m_instance = nullptr;
			vk::raii::SurfaceKHR m_surface = nullptr;
			std::unique_ptr<Device> m_device = nullptr;
			std::unique_ptr<Swapchain> m_swapchain = nullptr;
			vk::raii::DescriptorPool m_descriptorPool = nullptr;
			vk::raii::CommandPool m_commandPool = nullptr;
			std::array<std::unique_ptr<GBuffer>, MAX_FRAMES_IN_FLIGHT> m_gBuffers;

			vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;
			vk::PushConstantRange m_pushConstantRange;

			// Forward rendering
			vk::raii::PipelineLayout m_fwdPipelineLayout = nullptr;
			vk::raii::Pipeline m_fwdPipeline = nullptr;

			// Deferred rendering
			vk::raii::PipelineLayout m_defGeometryPipelineLayout = nullptr;
			vk::raii::Pipeline m_defGeometryPipeline = nullptr;
			vk::raii::PipelineLayout m_defLightingPipelineLayout = nullptr;
			vk::raii::Pipeline m_defLightingPipeline = nullptr;

			std::vector<vk::raii::CommandBuffer> m_commandBuffers;

			std::array<std::unique_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT> m_globalUBOs;
			std::array<std::unique_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT> m_objectSSBOs;
			std::vector<vk::raii::DescriptorSet> m_descriptorSets;

			std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
			std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
			std::vector<vk::raii::Fence> m_inFlightFences;
	};
}