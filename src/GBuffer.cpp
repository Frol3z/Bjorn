#include "GBuffer.hpp"

#include "Device.hpp"

namespace Felina
{
	GBuffer::GBuffer(const Device& device, vk::Extent2D extent)
        : m_extent(extent)
	{
        // TODO:
        // - create the g-buffer sampler
        // - figure out the descriptor set and layout of the g-buffer

        // NOTE:
        // - the extent should be updated when the framebuffer is resized

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        CreateAlbedoRenderTarget(device, allocCreateInfo);
        CreateSpecularRenderTarget(device, allocCreateInfo);
        CreateNormalRenderTarget(device, allocCreateInfo);
        CreateDepthRenderTarget(device, allocCreateInfo);
	}

    void GBuffer::CreateAlbedoRenderTarget(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
    {
        vk::ImageCreateInfo imageCreateInfo = {
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8G8B8A8Unorm,
            .extent = vk::Extent3D{ m_extent.width, m_extent.height, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };

        m_albedo = std::make_unique<Image>(device, imageCreateInfo, allocCreateInfo);
    }

    void GBuffer::CreateSpecularRenderTarget(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
    {
        vk::ImageCreateInfo imageCreateInfo{
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8G8B8A8Unorm, // RGB specular + A shininess
            .extent = vk::Extent3D{ m_extent.width, m_extent.height, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };

        m_specular = std::make_unique<Image>(device, imageCreateInfo, allocCreateInfo);
    }

    void GBuffer::CreateNormalRenderTarget(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
    {
        vk::ImageCreateInfo imageCreateInfo{
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR16G16B16A16Sfloat,
            .extent = vk::Extent3D{ m_extent.width, m_extent.height, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };

        m_normal = std::make_unique<Image>(device, imageCreateInfo, allocCreateInfo);
    }

    void GBuffer::CreateDepthRenderTarget(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
    {
        vk::ImageCreateInfo imageCreateInfo{
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eD32Sfloat,
            .extent = vk::Extent3D{ m_extent.width, m_extent.height, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };

        m_depth = std::make_unique<Image>(device, imageCreateInfo, allocCreateInfo);
    }
}