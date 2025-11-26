#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Felina
{	
	class Device;

	class PipelineBuilder
	{
		// DEFAULT CONFIGURATION:
		// - vertex input enabled
		// - backface culling enabled
		// - depth testing disabled

		public:
			struct ShaderStageInfo
			{
				// Path to SPIR-V compiled shader
				std::string shaderPath;
				vk::ShaderStageFlagBits stage;
				// `main` is assumed if not specified
				std::string entryPointName = "main";
			};

		public:
			PipelineBuilder(const Device& device);

			std::pair<vk::raii::Pipeline, vk::raii::PipelineLayout> BuildPipeline();
			void Reset();

			void SetShaderStages(const std::vector<ShaderStageInfo>& shaderStageInfos);
			void SetColorBlending(uint32_t colorAttachmentsCount);
			void SetPipelineLayout(
				const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, 
				const std::vector<vk::PushConstantRange>& pushConstantRanges
			);
			void SetAttachmentsFormat(const std::vector<vk::Format>& colorAttachmentsFormat, const vk::Format depthFormat);

			void EnableVertexInput();
			void DisableVertexInput();
			void EnableDepthTest();
			void DisableDepthTest();
			void EnableBackfaceCulling();
			void DisableBackfaceCulling();

		private:
			void InitDefaults();
			vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code) const;

			const Device& m_device;
		
			std::vector<vk::raii::ShaderModule> m_shaderModules; // no default
			std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStages; // no default
			std::vector<std::string> m_shaderEntryPoints;

			vk::PipelineVertexInputStateCreateInfo m_vertexInput{}; // default
			vk::VertexInputBindingDescription m_bindingDescription;
			std::vector<vk::VertexInputAttributeDescription> m_attributeDescriptions;

			vk::PipelineInputAssemblyStateCreateInfo m_inputAssembly{}; // default (will be configurable)

			vk::PipelineDynamicStateCreateInfo m_dynamic{}; // default
			std::vector<vk::DynamicState> m_dynamicStates;

			vk::PipelineViewportStateCreateInfo m_viewport{}; // default
			vk::PipelineRasterizationStateCreateInfo m_rasterizer{}; // default (configurable)
			vk::PipelineMultisampleStateCreateInfo m_multisample{}; // default (will be configurable)
			vk::PipelineDepthStencilStateCreateInfo m_depthStencil{}; // default (configurable)

			std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachments; // no default
			vk::PipelineColorBlendStateCreateInfo m_colorBlend{}; // no default

			vk::PipelineLayoutCreateInfo m_pipelineLayoutInfo{}; // no default
			std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
			std::vector<vk::PushConstantRange> m_pushConstantRanges;

			vk::PipelineRenderingCreateInfo m_rendering{}; // no default
			std::vector<vk::Format> m_colorAttachmentFormats;
			vk::Format m_depthFormat;
	};
}