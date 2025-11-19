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
			GBuffer(const Device& device, vk::Extent2D extent);

		private:
			void CreateAlbedoRenderTarget(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateSpecularRenderTarget(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateNormalRenderTarget(const Device& device, VmaAllocationCreateInfo allocCreateInfo);
			void CreateDepthRenderTarget(const Device& device, VmaAllocationCreateInfo allocCreateInfo);

			vk::Extent2D m_extent;

			std::unique_ptr<Image> m_albedo = nullptr;
			std::unique_ptr<Image> m_specular = nullptr;
			std::unique_ptr<Image> m_normal = nullptr;
			std::unique_ptr<Image> m_depth = nullptr;

			vk::Sampler m_sampler;
	};
}