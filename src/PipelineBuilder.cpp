#include "PipelineBuilder.hpp"

#include "Mesh.hpp"
#include "Device.hpp"
#include "Common.hpp"

namespace Felina
{
	PipelineBuilder::PipelineBuilder(const Device& device)
		: m_device(device)
	{
		InitDefaults();
	}

	std::pair<vk::raii::Pipeline, vk::raii::PipelineLayout> PipelineBuilder::BuildPipeline()
	{
		// Create pipeline layout
		auto pipelineLayout = vk::raii::PipelineLayout(m_device.GetDevice(), m_pipelineLayoutInfo);

		// Create graphic pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo{
			.pNext = &m_rendering,
			.stageCount = static_cast<uint32_t>(m_shaderStages.size()),
			.pStages = m_shaderStages.data(),
			.pVertexInputState = &m_vertexInput,
			.pInputAssemblyState = &m_inputAssembly,
			.pViewportState = &m_viewport,
			.pRasterizationState = &m_rasterizer,
			.pMultisampleState = &m_multisample,
			.pDepthStencilState = &m_depthStencil,
			.pColorBlendState = &m_colorBlend,
			.pDynamicState = &m_dynamic,
			.layout = pipelineLayout,
			.renderPass = nullptr // Because we are using dynamic rendering
		};
		auto pipeline = vk::raii::Pipeline(m_device.GetDevice(), nullptr, pipelineInfo);

		return { std::move(pipeline), std::move(pipelineLayout) };
	}

	void PipelineBuilder::Reset()
	{
		// Destroy shader modules first (RAII automatically)
		m_shaderModules.clear();
		m_shaderStages.clear();
		m_shaderEntryPoints.clear();

		// Color blending
		m_colorBlendAttachments.clear();
		m_colorBlend.attachmentCount = 0;
		m_colorBlend.pAttachments = nullptr;

		// Pipeline layout info
		m_descriptorSetLayouts.clear();
		m_pushConstantRanges.clear();
		m_pipelineLayoutInfo.setLayoutCount = 0;
		m_pipelineLayoutInfo.pSetLayouts = nullptr;
		m_pipelineLayoutInfo.pushConstantRangeCount = 0;
		m_pipelineLayoutInfo.pPushConstantRanges = nullptr;

		// Attachments
		m_colorAttachmentFormats.clear();
		m_rendering.colorAttachmentCount = 0;
		m_rendering.pColorAttachmentFormats = nullptr;
		m_rendering.depthAttachmentFormat = vk::Format::eUndefined;

		// Reinitialize with defaults
		InitDefaults();
	}

	void PipelineBuilder::SetShaderStages(const std::vector<ShaderStageInfo>& shaderStageInfos)
	{
		// Reserve the correct amount of space to avoid reallocation
		size_t count = shaderStageInfos.size();
		m_shaderStages.reserve(count);
		m_shaderModules.reserve(count);
		m_shaderEntryPoints.reserve(count);

		for (const auto& info : shaderStageInfos)
		{
			// Create and retain a reference to the shader module
			m_shaderModules.emplace_back(CreateShaderModule(ReadFile(info.shaderPath)));
			m_shaderEntryPoints.push_back(info.entryPointName);
			auto& epName = m_shaderEntryPoints.back();
			auto& shaderModule = m_shaderModules.back();

			// Create shader stage create info
			m_shaderStages.emplace_back(vk::PipelineShaderStageCreateInfo{
				.stage = info.stage,
				.module = shaderModule,
				.pName = epName.c_str()
			});
		}
	}

	void PipelineBuilder::SetColorBlending(uint32_t colorAttachmentsCount)
	{
		// Color blending disabled for now but still needs to be setup
		m_colorBlendAttachments.resize(colorAttachmentsCount);
		for (auto& attachment : m_colorBlendAttachments)
		{
			attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
			attachment.blendEnable = vk::False;
		}

		// Blending with bitwise operations
		m_colorBlend.logicOpEnable = vk::False;
		m_colorBlend.logicOp = vk::LogicOp::eCopy;
		m_colorBlend.attachmentCount = static_cast<uint32_t>(m_colorBlendAttachments.size());
		m_colorBlend.pAttachments = m_colorBlendAttachments.data();
	}

	void PipelineBuilder::SetPipelineLayout(
		const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
		const std::vector<vk::PushConstantRange>& pushConstantRanges
	)
	{	
		// create a local copy until the pipeline is built
		m_descriptorSetLayouts = descriptorSetLayouts;
		m_pushConstantRanges = pushConstantRanges;

		m_pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayouts.size());
		m_pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();
		m_pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(m_pushConstantRanges.size());
		m_pipelineLayoutInfo.pPushConstantRanges = m_pushConstantRanges.data();
	}

	void PipelineBuilder::SetAttachmentsFormat(const std::vector<vk::Format>& colorAttachmentsFormat, const vk::Format depthFormat)
	{
		// create a local copy until the pipeline is built
		m_colorAttachmentFormats = colorAttachmentsFormat;
		m_depthFormat = depthFormat;

		m_rendering.colorAttachmentCount = static_cast<uint32_t>(m_colorAttachmentFormats.size());
		m_rendering.pColorAttachmentFormats = m_colorAttachmentFormats.data();
		m_rendering.depthAttachmentFormat = m_depthFormat;
	}

	void PipelineBuilder::EnableVertexInput()
	{
		m_bindingDescription = Vertex::GetBindingDescription();
		auto arr = Vertex::GetAttributeDescriptions();
		m_attributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(arr.begin(), arr.end());
		m_vertexInput.vertexBindingDescriptionCount = 1;
		m_vertexInput.pVertexBindingDescriptions = &m_bindingDescription;
		m_vertexInput.vertexAttributeDescriptionCount = m_attributeDescriptions.size();
		m_vertexInput.pVertexAttributeDescriptions = m_attributeDescriptions.data();
	}

	void PipelineBuilder::DisableVertexInput()
	{
		m_vertexInput.vertexBindingDescriptionCount = 0;
		m_vertexInput.pVertexBindingDescriptions = nullptr;
		m_vertexInput.vertexAttributeDescriptionCount = 0;
		m_vertexInput.pVertexAttributeDescriptions = nullptr;
	}

	void PipelineBuilder::EnableDepthTest()
	{
		m_depthStencil.depthTestEnable = vk::True;
		m_depthStencil.depthWriteEnable = vk::True;
	}

	void PipelineBuilder::DisableDepthTest()
	{
		m_depthStencil.depthTestEnable = vk::False;
		m_depthStencil.depthWriteEnable = vk::False;
	}

	void PipelineBuilder::EnableBackfaceCulling()
	{
		m_rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	}

	void PipelineBuilder::DisableBackfaceCulling()
	{
		m_rasterizer.cullMode = vk::CullModeFlagBits::eNone;
	}

	void PipelineBuilder::InitDefaults()
	{
		// VERTEX INPUT
		EnableVertexInput();

		// INPUT ASSEMBLY
		m_inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

		// DYNAMIC STATES (viewport and scissor)
		// Viewport and scissors placeholders (will be set through the command buffer at draw time)
		m_dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		m_dynamic.dynamicStateCount = static_cast<uint32_t>(m_dynamicStates.size());
		m_dynamic.pDynamicStates = m_dynamicStates.data();

		// VIEWPORT
		// We just need to specify how many there are when creating the pipeline
		m_viewport.viewportCount = 1;
		m_viewport.scissorCount = 1;

		// RASTERIZER
		m_rasterizer.depthClampEnable = vk::False; // Clamp out of bound depth values to the near or far plane
		m_rasterizer.rasterizerDiscardEnable = vk::False; // Disable rasterization (no output)
		m_rasterizer.polygonMode = vk::PolygonMode::eFill; // For wireframe mode a GPU feature must be enabled
		m_rasterizer.cullMode = vk::CullModeFlagBits::eBack; // Backface culling as DEFAULT
		m_rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
		m_rasterizer.depthBiasEnable = vk::False;
		m_rasterizer.depthBiasSlopeFactor = 1.0f;
		m_rasterizer.lineWidth = 1.0f;

		// MSAA (disabled by default)
		m_multisample.rasterizationSamples = vk::SampleCountFlagBits::e1;
		m_multisample.sampleShadingEnable = vk::False;

		// DEPTH and STENCIL (disabled by default)
		m_depthStencil.depthTestEnable = vk::False;
		m_depthStencil.depthWriteEnable = vk::False; // Enable writing to the depth attachment
		m_depthStencil.depthCompareOp = vk::CompareOp::eLess; // If fragment < depth then the test is passed (+Z forward)
		m_depthStencil.depthBoundsTestEnable = vk::False;
		m_depthStencil.stencilTestEnable = vk::False; // Stencil test disabled for now
	}

	vk::raii::ShaderModule PipelineBuilder::CreateShaderModule(const std::vector<char>& code) const
	{
		vk::ShaderModuleCreateInfo createInfo{
			.codeSize = code.size() * sizeof(char),
			.pCode = reinterpret_cast<const uint32_t*>(code.data())
		};
		vk::raii::ShaderModule shaderModule(m_device.GetDevice(), createInfo);
		return shaderModule;
	}
}