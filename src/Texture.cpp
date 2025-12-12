#include "Texture.hpp"

#include "Device.hpp"

namespace Felina
{
	Texture::Texture(const Device& device, const vk::ImageCreateInfo& imageInfo, const VmaAllocationCreateInfo& allocInfo)
		: m_allocator(device.GetAllocator())
	{
		// Image
		VkImage raw = VK_NULL_HANDLE;
		VkResult res = vmaCreateImage(
			m_allocator,
			reinterpret_cast<const VkImageCreateInfo*>(&imageInfo),
			&allocInfo,
			&raw,
			&m_allocation,
			nullptr
		);
		if (res != VK_SUCCESS)
			throw std::runtime_error("[IMAGE] vmaCreateImage: " + std::to_string(res));

		m_imageCreateInfo = imageInfo;
		m_image = vk::Image(raw);

		// Image View
		CreateImageView(device);
	}

	Texture::~Texture()
	{
		if (m_image && m_allocation)
		{
			m_imageView = nullptr;
			vmaDestroyImage(m_allocator, m_image, m_allocation);
			m_image = nullptr;
			m_allocation = VK_NULL_HANDLE;
		}
	}

	void Texture::CreateImageView(const Device& device)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.image = m_image;
		imageViewCreateInfo.format = m_imageCreateInfo.format;

		// .viewType
		// NOTE: may be updated/moved if more complex combinations are needed
		switch (m_imageCreateInfo.imageType)
		{
			case vk::ImageType::e1D:
				imageViewCreateInfo.viewType =
					m_imageCreateInfo.arrayLayers > 1 ?
					vk::ImageViewType::e1DArray :
					vk::ImageViewType::e1D;
				break;

			case vk::ImageType::e2D:
				imageViewCreateInfo.viewType =
					m_imageCreateInfo.arrayLayers > 1 ?
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
		if (m_imageCreateInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		{
			// Add depth buffer
			aspect |= vk::ImageAspectFlagBits::eDepth;
			
			// Add stencil buffer as well if needed
			if (m_imageCreateInfo.format == vk::Format::eD32SfloatS8Uint || m_imageCreateInfo.format == vk::Format::eD24UnormS8Uint)
				aspect |= vk::ImageAspectFlagBits::eStencil;
		}
		else
		{
			aspect |= vk::ImageAspectFlagBits::eColor;
		}

		imageViewCreateInfo.subresourceRange = vk::ImageSubresourceRange{
			aspect,
			0,						// baseMipLevel                
			m_imageCreateInfo.mipLevels,
			0,						// baseArrayLayer                
			m_imageCreateInfo.arrayLayers
		};

		m_imageViewCreateInfo = imageViewCreateInfo;
		m_imageView = vk::raii::ImageView(device.GetDevice(), imageViewCreateInfo);
	}
}