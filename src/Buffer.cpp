#include "Buffer.hpp"

namespace Bjorn
{
	Buffer::Buffer(
		const VkBufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocInfo, 
		const VmaAllocator& allocator, const vk::raii::Device& device
	)
		: m_allocator(allocator)
	{
		vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
	}

	Buffer::~Buffer()
	{
		if (m_buffer || m_allocation)
		{
			vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
		}
	}
	
	void Buffer::LoadData(const void* data, const size_t size)
	{
		// TODO: Assert if we should be able to load data onto this buffer
		void* mappedMemory = nullptr;
		vmaMapMemory(m_allocator, m_allocation, &mappedMemory);
		memcpy(mappedMemory, data, size);
		vmaUnmapMemory(m_allocator, m_allocation);
	}
}