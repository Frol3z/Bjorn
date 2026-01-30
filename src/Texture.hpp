#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

namespace Felina
{
	class Device;

	class Texture
	{
		public:
			Texture(const Device& device,
				const vk::ImageCreateInfo& imageInfo,
				const VmaAllocationCreateInfo& allocInfo
			);
			~Texture();

			const bool IsCubemap() {
				return (m_imageCreateInfo.imageType == vk::ImageType::e2D)
					&& (m_imageCreateInfo.flags & vk::ImageCreateFlagBits::eCubeCompatible)
					&& (m_imageCreateInfo.arrayLayers == 6);
			}
			const vk::Image& GetHandle() const { return m_image; };
			const vk::raii::ImageView& GetImageView() const { return m_imageView; }
			const vk::Format GetFormat() const { return m_imageCreateInfo.format; }
			const vk::ImageSubresourceRange GetImageSubresourceRange() const { return m_imageViewCreateInfo.subresourceRange; }
			const vk::Extent3D GetExtent() const { return m_imageCreateInfo.extent; }

		private:
			void CreateImageView(const Device& device);

			const VmaAllocator& m_allocator;
			vk::Image m_image; // no RAII because of VMA
			vk::raii::ImageView m_imageView = nullptr;
			VmaAllocation m_allocation;

			vk::ImageCreateInfo m_imageCreateInfo;
			vk::ImageViewCreateInfo m_imageViewCreateInfo;
	};
}