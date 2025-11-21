#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

namespace Felina
{
	class Device;

	class Image
	{
		public:
			Image(const Device& device,
				const vk::ImageCreateInfo& imageInfo,
				const VmaAllocationCreateInfo& allocInfo
			);
			~Image();

			const vk::Image& GetHandle() const { return m_image; };
			const vk::raii::ImageView& GetImageView() const { return m_imageView; }
			const vk::Format GetFormat() const { return m_format; }

		private:
			void CreateImageView(const Device& device, const vk::ImageCreateInfo& imageInfo);

			const VmaAllocator& m_allocator;
			vk::Image m_image; // no RAII because of VMA
			vk::Format m_format;
			vk::raii::ImageView m_imageView = nullptr;
			VmaAllocation m_allocation;
	};
}