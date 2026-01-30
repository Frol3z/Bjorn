#include "GBuffer.hpp"

#include "Device.hpp"

namespace Felina
{
	GBuffer::GBuffer(
        const Device& device,
        vk::Extent2D swapchainExtent,
        vk::raii::DescriptorPool& descriptorPool
    )
        : m_extent(swapchainExtent)
	{
        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        CreateBaseColorAttachment(device, allocCreateInfo);
        CreateMaterialInfoAttachment(device, allocCreateInfo);
        CreateNormalAttachment(device, allocCreateInfo);
        CreateDepthAttachment(device, allocCreateInfo);

        CreateSampler(device);

        CreateDescriptorSetLayout(device);
        CreateDescriptorSets(device, descriptorPool);
	}

    std::vector<vk::Format> GBuffer::GetColorAttachmentFormats() const
    {
        std::vector<vk::Format> m_formats;
        for (const auto& attachment : m_attachments)
        {
            if (attachment.type == AttachmentType::Depth)
                continue;

            m_formats.push_back(attachment.image->GetFormat());
        }
        return m_formats;
    }

    vk::Format GBuffer::GetDepthFormat() const
    {
        for (const auto& attachment : m_attachments)
        {
            if (attachment.type == AttachmentType::Depth)
                return attachment.image->GetFormat();
        }
        throw std::runtime_error("[GBUFFER] Couldn't retrieve the G-buffer depth attachment format because there's no such attachment!");
    }

    void GBuffer::Recreate(
        const Device& device,
        vk::Extent2D swapchainExtent,
        vk::raii::DescriptorPool& descriptorPool
    )
    {
        // Clean up the "old" G-buffer
        CleanUp();
        
        // Update local extent with the resized swapchain extent
        m_extent = swapchainExtent;

        // Recreate images and descriptor sets
        // NOTE: sampler and descriptor sets layout DO NOT need to be recreated
        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        CreateBaseColorAttachment(device, allocCreateInfo);
        CreateMaterialInfoAttachment(device, allocCreateInfo);
        CreateNormalAttachment(device, allocCreateInfo);
        CreateDepthAttachment(device, allocCreateInfo);
        CreateDescriptorSets(device, descriptorPool);
    }

    void GBuffer::CreateBaseColorAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
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
            AttachmentType::BaseColor,
            std::make_unique<Texture>(device, imageCreateInfo, allocCreateInfo)
        });
    }

    void GBuffer::CreateMaterialInfoAttachment(const Device& device, VmaAllocationCreateInfo allocCreateInfo)
    {
        vk::ImageCreateInfo imageCreateInfo{
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
            AttachmentType::MaterialInfo,
            std::make_unique<Texture>(device, imageCreateInfo, allocCreateInfo)
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
            std::make_unique<Texture>(device, imageCreateInfo, allocCreateInfo)
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
            std::make_unique<Texture>(device, imageCreateInfo, allocCreateInfo)
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

    void GBuffer::CreateDescriptorSets(const Device& device, vk::raii::DescriptorPool& descriptorPool)
    {
        // Allocate descriptor sets
        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &*m_descriptorSetLayout
        };
        m_descriptorSet = std::move(device.GetDevice().allocateDescriptorSets(allocInfo)[0]);

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
                .dstSet = m_descriptorSet,
                .dstBinding = static_cast<uint32_t>(j),
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &imageInfos[j]
            };
        }
        device.GetDevice().updateDescriptorSets(descriptorWrites, {});
    }

    void GBuffer::CleanUp()
    {
        m_descriptorSet = nullptr;
        m_attachments.clear();
    }
}