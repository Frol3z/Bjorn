#include "GBuffer.hpp"

#include "Device.hpp"

#include <iostream>

namespace Felina
{
	GBuffer::GBuffer(
        const Device& device,
        vk::Extent2D extent,
        vk::raii::DescriptorPool& descriptorPool,
        const uint32_t maxFramesInFlight
    )
        : m_extent(extent)
	{
        // NOTE:
        // - the extent should be updated when the framebuffer is resized

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        CreateAlbedoAttachment(device, allocCreateInfo);
        CreateSpecularAttachment(device, allocCreateInfo);
        CreateNormalAttachment(device, allocCreateInfo);
        CreateDepthAttachment(device, allocCreateInfo);

        CreateSampler(device);

        CreateDescriptorSetLayout(device);
        CreateDescriptorSets(device, descriptorPool, maxFramesInFlight);
	}

    void GBuffer::CreateAlbedoAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
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

        m_attachments.push_back({
            AttachmentType::Albedo,
            std::make_unique<Image>(device, imageCreateInfo, allocCreateInfo)
        });
    }

    void GBuffer::CreateSpecularAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
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

        m_attachments.push_back({ 
            AttachmentType::Specular,
            std::make_unique<Image>(device, imageCreateInfo, allocCreateInfo) 
        });
    }

    void GBuffer::CreateNormalAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
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

        m_attachments.push_back({
            AttachmentType::Normal,
            std::make_unique<Image>(device, imageCreateInfo, allocCreateInfo)
        });
    }

    void GBuffer::CreateDepthAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
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

        m_attachments.push_back({
            AttachmentType::Depth,
            std::make_unique<Image>(device, imageCreateInfo, allocCreateInfo)
        });
    }

    void GBuffer::CreateSampler(const Device& device)
    {
        vk::SamplerCreateInfo samplerCreateInfo{
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eNearest,
            .addressModeU = vk::SamplerAddressMode::eClampToEdge, // this guarantees stability on the borders of the image
            .addressModeV = vk::SamplerAddressMode::eClampToEdge,
            .addressModeW = vk::SamplerAddressMode::eClampToEdge,
            .mipLodBias = 0.0f,
            .anisotropyEnable = vk::False,
            .maxAnisotropy = 1.0f,
            .compareEnable = vk::False,
            .compareOp = vk::CompareOp::eAlways,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = vk::BorderColor::eFloatOpaqueBlack,
            .unnormalizedCoordinates = vk::False,
        };
        m_sampler = vk::raii::Sampler(device.GetDevice(), samplerCreateInfo);
    }

    void GBuffer::CreateDescriptorSetLayout(const Device& device)
    {
        // Bindings
        std::vector<vk::DescriptorSetLayoutBinding> bindings{};
        bindings.resize(m_attachments.size());
        for (size_t i = 0; i < m_attachments.size(); i++)
        {
            bindings[i] = vk::DescriptorSetLayoutBinding{
                .binding = static_cast<uint32_t>(i),
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment,
                .pImmutableSamplers = nullptr
            };
        }

        // Create descriptor set layout
        vk::DescriptorSetLayoutCreateInfo layoutInfo{
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };
        m_descriptorSetLayout = vk::raii::DescriptorSetLayout(device.GetDevice(), layoutInfo);
    }

    void GBuffer::CreateDescriptorSets(const Device& device, vk::raii::DescriptorPool& descriptorPool, const uint32_t maxFramesInFlight)
    {
        // Allocate descriptor sets
        std::vector<vk::DescriptorSetLayout> layouts(maxFramesInFlight, m_descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool = descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
            .pSetLayouts = layouts.data()
        };
        m_descriptorSets = device.GetDevice().allocateDescriptorSets(allocInfo);

        // Bind the images to the descriptor sets
        for (size_t i = 0; i < maxFramesInFlight; i++)
        {
            std::vector<vk::DescriptorImageInfo> imageInfos(m_attachments.size());
            std::vector<vk::WriteDescriptorSet> descriptorWrites(m_attachments.size());
            for (size_t j = 0; j < m_attachments.size(); j++)
            {
                // DescriptorImageInfo
                imageInfos[j] = vk::DescriptorImageInfo{
                    .sampler = m_sampler,
                    .imageView = m_attachments[j].image->GetImageView(),
                    .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                };

                // WriteDescriptorSet
                descriptorWrites[j] = vk::WriteDescriptorSet{
                    .dstSet = m_descriptorSets[i],
                    .dstBinding = static_cast<uint32_t>(j),
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                    .pImageInfo = &imageInfos[j]
                };
            }
            device.GetDevice().updateDescriptorSets(descriptorWrites, {});
        }
    }
}