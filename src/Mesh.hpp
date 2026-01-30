#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace Felina 
{
	class Buffer;
	class Device;

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;

		static vk::VertexInputBindingDescription GetBindingDescription()
		{
			return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
		}

		static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions()
		{
			return
			{
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv))
			};
		}
	};

	class Mesh
	{
		public:
			enum Type { CUBE = 0, SPHERE };
		public:
			Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
			Mesh(Mesh::Type type); // Procedurally generate a mesh based on type
			~Mesh();

			void Load(Device& device);
			void Unload();
			
			const Buffer& GetVertexBuffer() const { 
				assert(m_vertexBuffer && "Vertex buffer not initialized");
				return *m_vertexBuffer;
			};
			const Buffer& GetIndexBuffer() const { 
				assert(m_indexBuffer && "Index buffer not initialized");
				return *m_indexBuffer;
			};

			size_t GetIndexBufferSize() const { return m_indices.size(); };
			vk::IndexType GetIndexType() const { return vk::IndexType::eUint32; };

		private:
			void CreateCubeMesh();
			void CreateSphereMesh(uint32_t nSlices = 32, uint32_t nStacks = 32);

			void CreateStagingBuffer(Device& device, vk::DeviceSize size);
			void DestroyStagingBuffer();
			void CreateVertexBuffer(Device& device, vk::DeviceSize size);
			void CreateIndexBuffer(Device& device, vk::DeviceSize size);

			std::vector<Vertex> m_vertices;
			std::vector<uint32_t> m_indices;

			std::unique_ptr<Buffer> m_stagingBuffer = nullptr;
			std::unique_ptr<Buffer> m_vertexBuffer = nullptr;
			std::unique_ptr<Buffer> m_indexBuffer = nullptr;
	};
}