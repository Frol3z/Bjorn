#include "Image.hpp"

#include "Device.hpp"

namespace Felina
{
	Image::Image(const Device& device, const vk::ImageCreateInfo& imageInfo, const VmaAllocationCreateInfo& allocInfo)
		: m_allocator(device.GetAllocator())
	{
		// Image
		VkImage raw;
		vmaCreateImage(m_allocator, &*imageInfo, &allocInfo, &raw, &m_allocation, nullptr);
		m_image = vk::Image(raw);

		// Image View
		CreateImageView(device, imageInfo);
	}

	Image::~Image()
	{
		if (m_image && m_allocation)
		{
			m_imageView = nullptr;
			vmaDestroyImage(m_allocator, m_image, m_allocation);
			m_image = nullptr;
			m_allocation = VK_NULL_HANDLE;
		}
	}

	void Image::CreateImageView(const Device& device, const vk::ImageCreateInfo& imageInfo)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.image = m_image;
		imageViewCreateInfo.format = imageInfo.format;

		// .viewType
		// NOTE: may be updated/moved if more complex combinations are needed
		switch (imageInfo.imageType)
		{
			case vk::ImageType::e1D:
				imageViewCreateInfo.viewType =
					imageInfo.arrayLayers > 1 ?
					vk::ImageViewType::e1DArray :
					vk::ImageViewType::e1D;
				break;

			case vk::ImageType::e2D:
				imageViewCreateInfo.viewType =
					imageInfo.arrayLayers > 1 ?
					vk::ImageViewType::e2DArray :
					vk::ImageViewType::e2D;
				break;

			case vk::ImageType::e3D:
				// 3D images use e3D view type
				imageViewCreateInfo.viewType = vk::ImageViewType::e3D;
				break;
		}

		// .subresourceRange
		vk::ImageAspectFlags aspect{};
		if (imageInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		{
			// Add depth buffer
			aspect |= vk::ImageAspectFlagBits::eDepth;
			
			// Add stencil buffer as well if needed
			if (imageInfo.format == vk::Format::eD32SfloatS8Uint || imageInfo.format == vk::Format::eD24UnormS8Uint)
				aspect |= vk::ImageAspectFlagBits::eStencil;
		}
		else
		{
			aspect |= vk::ImageAspectFlagBits::eColor;
		}

		imageViewCreateInfo.subresourceRange = vk::ImageSubresourceRange{
			aspect,
			0,						// baseMipLevel                
			imageInfo.mipLevels,
			0,						// baseArrayLayer                
			imageInfo.arrayLayers
		};

		m_imageView = vk::raii::ImageView(device.GetDevice(), imageViewCreateInfo);
	}
}