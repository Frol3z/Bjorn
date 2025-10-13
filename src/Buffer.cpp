#include "Buffer.hpp"

namespace Bjorn
{
	Buffer::Buffer(const VkBufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocInfo, const VmaAllocator& allocator)
		: m_allocator(allocator)
	{
		VkBuffer raw;
		vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &raw, &m_allocation, nullptr);
		m_buffer = vk::Buffer(raw);
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