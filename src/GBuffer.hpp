#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

#include "Image.hpp"

namespace Felina
{
	class Image;
	class Device;

	class GBuffer
	{
		public:
			GBuffer(
				const Device& device, 
				vk::Extent2D swapchainExtent,
				vk::raii::DescriptorPool& descriptorPool,
				const uint32_t maxFramesInFlight
			);

			void Recreate(
				const Device& device,
				vk::Extent2D swapchainExtent,
				vk::raii::DescriptorPool& descriptorPool,
				const uint32_t maxFramesInFlight
			);

		private:
			enum AttachmentType {Albedo = 0, Specular, Normal, Depth};
			struct Attachment
			{
				AttachmentType type;
				std::unique_ptr<Image> image = nullptr;
			};

			void CreateAlbedoAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateSpecularAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateNormalAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateDepthAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateSampler(const Device& device);
			void CreateDescriptorSetLayout(const Device& device);
			void CreateDescriptorSets(const Device& device, vk::raii::DescriptorPool& descriptorPool, const uint32_t maxFramesInFlight);
			void CleanUp();

			vk::Extent2D m_extent;
			std::vector<Attachment> m_attachments;
			vk::raii::Sampler m_sampler = nullptr;
			vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;
			std::vector<vk::raii::DescriptorSet> m_descriptorSets;
	};
}