#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

namespace Bjorn
{
	class Device
	{
	public:
		Device(vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);
		~Device();

		const vk::raii::Device& GetDevice() const { return m_device; }
		const vk::raii::PhysicalDevice& GetPhysicalDevice() const { return m_physicalDevice; }
		const VmaAllocator& GetAllocator() const { return m_allocator; }
		const vk::raii::Queue& GetGraphicsQueue() const { return m_graphicsQueue; }
		uint32_t GetGraphicsQueueFamilyIndex() const { return m_graphicsQueueFamilyIndex; }
		const vk::raii::Queue& GetPresentQueue() const { return m_presentQueue; }
		uint32_t GetPresentQueueFamilyIndex() const { return m_presentQueueFamilyIndex; }

	private:
		void SelectPhysicalDevice(vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);
		void CreateLogicalDevice(vk::raii::Instance& instance);
		void CreateMemoryAllocator(vk::raii::Instance& instance);

		vk::raii::PhysicalDevice m_physicalDevice = nullptr;
		vk::raii::Device m_device = nullptr;
		
		VmaAllocator m_allocator;
		
		vk::raii::Queue m_graphicsQueue = nullptr;
		vk::raii::Queue m_presentQueue = nullptr;
		uint32_t m_graphicsQueueFamilyIndex;
		uint32_t m_presentQueueFamilyIndex;
	};
}