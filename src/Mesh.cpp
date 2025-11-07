#include "Mesh.hpp"

//
// The workflow for loading a Mesh should be:
// - the Application will tell the Renderer to load a specific Mesh
// - in addition, it will specify how to load the Mesh (from file, passing data from the Application level, etc...)
// - the Renderer will take a reference to this Mesh and all the required data/info on how to load it
// - then it will invoke the appropriate load method on the Mesh by passing references of any backend specific dependency
//   e.g. VMAallocator, etc...
// - the Mesh will load required data on CPU memory and request to transfer data to the GPU back to the Renderer
// - the Renderer will then execute the request (via command buffer submitter to the GPU)
// - once finished, the Renderer will tell the Mesh that it's free to deallocate the staging buffer which is not needed anymore
// - once finished, the Renderer should notify the Application of the result
// 
// TODO - figure out where to store and how to send meshes to be drawn to the Renderer
// - if the Mesh was loaded successfully then it will be stored **somewhere** where it will be requested at drawing time again
//   by the Renderer
// NOTE - consider implementing frustum and occlusion culling in the future
//

namespace Bjorn
{
    Mesh::Mesh()
    {
        // TODO - remove this hardcoded data into a load from file or passed from the application functionality
        m_vertices =
        {
            {{-0.5f, -0.5f,  0.0f}, {1.0f, 1.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.0f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}}
        };

        // NOTE - Change index type in the binding if modified
        m_indices = {
            0, 1, 2, 2, 3, 0
        };
    }

    Mesh::~Mesh()
    {
        Unload();
    }

	void Mesh::Load(const VmaAllocator& allocator, Renderer& renderer)
	{
        auto vertexDataSize = m_vertices.size() * sizeof(Vertex);
        if (!m_indices.empty())
        {
            // The mesh supports indices
            auto indexDataSize = m_indices.size() * sizeof(m_indices[0]);

            // stagingSize should be equal to vertexDataSize most of the time
            auto stagingSize = std::max(vertexDataSize, indexDataSize);
            CreateStagingBuffer(allocator, stagingSize);
            CreateVertexBuffer(allocator, vertexDataSize, renderer);
            CreateIndexBuffer(allocator, indexDataSize, renderer);
        }
        else
        {
            // The mesh does not support indices -> vertices only representation
            CreateStagingBuffer(allocator, vertexDataSize);
            CreateVertexBuffer(allocator, vertexDataSize, renderer);
        }

        // Once this line is reached the data has been transferred
        // correctly to GPU memory, so the staging buffer can be safely destroyed
        // since it won't be needed anymore
        DestroyStagingBuffer();
	}

    void Mesh::Unload()
    {
        m_vertexBuffer.reset();
        m_indexBuffer.reset();
    }

    void Mesh::CreateStagingBuffer(const VmaAllocator& allocator, vk::DeviceSize size)
    {        
        VkBufferCreateInfo stagingInfo{};
        stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingInfo.size = size;
        stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo{};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        m_stagingBuffer = std::make_unique<Buffer>(stagingInfo, stagingAllocInfo, allocator);
    }

    void Mesh::DestroyStagingBuffer()
    {
        m_stagingBuffer.reset();
    }

    void Mesh::CreateVertexBuffer(const VmaAllocator& allocator, vk::DeviceSize size, Renderer & renderer)
    {
        VkBufferCreateInfo vertexInfo{};
        vertexInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexInfo.size = size;
        vertexInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        vertexInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo vertexAllocInfo{};
        vertexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        m_vertexBuffer = std::make_unique<Buffer>(vertexInfo, vertexAllocInfo, allocator);

        // Load vertex data on the staging buffer
        m_stagingBuffer->LoadData(m_vertices.data(), size);

        // Issue request to copy vertex data loaded on the staging buffer to GPU memory
        renderer.CopyBuffer(*m_stagingBuffer, *m_vertexBuffer, size);
    }

    void Mesh::CreateIndexBuffer(const VmaAllocator& allocator, vk::DeviceSize size, Renderer& renderer)
    {
        VkBufferCreateInfo indexInfo{};
        indexInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexInfo.size = size;
        indexInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        indexInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo indexAllocInfo{};
        indexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        m_indexBuffer = std::make_unique<Buffer>(indexInfo, indexAllocInfo, allocator);

        // Load index data on the staging buffer
        m_stagingBuffer->LoadData(m_indices.data(), size);

        // Issue request to copy index data loaded on the staging buffer to GPU memory
        renderer.CopyBuffer(*m_stagingBuffer, *m_indexBuffer, size);
    }
}