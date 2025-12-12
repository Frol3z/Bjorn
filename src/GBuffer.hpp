#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

#include "Texture.hpp"

namespace Felina
{
	class Texture;
	class Device;

	class GBuffer
	{
		public:
			enum AttachmentType { Albedo = 0, Specular, MaterialInfo, Normal, Depth };
			struct Attachment
			{
				AttachmentType type;
				std::unique_ptr<Texture> image = nullptr;
			};

			GBuffer(
				const Device& device, 
				vk::Extent2D swapchainExtent,
				vk::raii::DescriptorPool& descriptorPool
			);

			const std::vector<Attachment>& GetAttachments() const { return m_attachments; }
			size_t GetAttachmentsCount() const { return m_attachments.size(); }
			std::vector<vk::Format> GetColorAttachmentFormats() const;
			vk::Format GetDepthFormat() const;
			const vk::raii::DescriptorSetLayout& GetDescriptorSetLayout() const { return m_descriptorSetLayout; }
			const vk::raii::DescriptorSet& GetDescriptorSet() const { return m_descriptorSet; }

			void Recreate(
				const Device& device,
				vk::Extent2D swapchainExtent,
				vk::raii::DescriptorPool& descriptorPool
			);

		private:
			void CreateAlbedoAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateSpecularAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateMaterialInfoAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateNormalAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateDepthAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateSampler(const Device& device);
			void CreateDescriptorSetLayout(const Device& device);
			void CreateDescriptorSets(const Device& device, vk::raii::DescriptorPool& descriptorPool);
			void CleanUp();

			vk::Extent2D m_extent;
			std::vector<Attachment> m_attachments;
			vk::raii::Sampler m_sampler = nullptr;
			vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;
			vk::raii::DescriptorSet m_descriptorSet = nullptr;
	};
}