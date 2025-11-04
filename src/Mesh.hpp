#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Renderer.hpp"
#include "Buffer.hpp"

namespace Bjorn 
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;

		static vk::VertexInputBindingDescription GetBindingDescription()
		{
			return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
		}

		static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions()
		{
			return
			{
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))
			};
		}
	};

	class Mesh
	{
		public:

			Mesh() = default;
			~Mesh();

			bool Load(const VmaAllocator& allocator, Renderer& renderer);

			// For the future
			//bool LoadFromFile();

			// Free up buffers from GPU memory
			//bool Unload();
			
			// Getters for the buffers handles to be able to allow binding before the draw call
			const Buffer& GetVertexBuffer() const { 
				assert(m_vertexBuffer && "Vertex buffer not initialized");
				return *m_vertexBuffer;
			};

			const Buffer& GetIndexBuffer() const { 
				assert(m_indexBuffer && "Index buffer not initialized");
				return *m_indexBuffer;
			};

			size_t GetIndexBufferSize() const {
				return m_indices.size();
			};

		private:
			void CreateStagingBuffer(const VmaAllocator& allocator, vk::DeviceSize size);
			void DestroyStagingBuffer();
			void CreateVertexBuffer(const VmaAllocator& allocator, vk::DeviceSize size,Renderer& renderer);
			void CreateIndexBuffer(const VmaAllocator& allocator, vk::DeviceSize size,Renderer& renderer);

			std::vector<Vertex> m_vertices;
			std::vector<uint16_t> m_indices;

			std::unique_ptr<Buffer> m_stagingBuffer = nullptr;
			std::unique_ptr<Buffer> m_vertexBuffer = nullptr;
			std::unique_ptr<Buffer> m_indexBuffer = nullptr;
	};
}