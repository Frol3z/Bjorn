#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

namespace Bjorn
{
	class Buffer
	{
	public:
		Buffer(
			const VkBufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocInfo, 
			const VmaAllocator& allocator, const vk::raii::Device& device
		);
		~Buffer();

		void LoadData(const void* data, const size_t size);
		const VkBuffer& GetHandle() const { return m_buffer; };

	private:
		const VmaAllocator& m_allocator;
		VkBuffer m_buffer = nullptr;
		VmaAllocation m_allocation;
	};
}