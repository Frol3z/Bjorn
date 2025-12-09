#include "Mesh.hpp"

#include "Device.hpp"
#include "Buffer.hpp"

namespace Felina
{
    Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
        : m_vertices(vertices), m_indices(indices)
    {
    }

    Mesh::Mesh(Mesh::Type type)
    {
        switch (type)
        {
            case CUBE: 
                CreateCubeMesh();
                break;
            case SPHERE:
                CreateSphereMesh();
                break;
            default:
                break;
        }
    }

    Mesh::~Mesh()
    {
        Unload();
    }

	void Mesh::Load(Device& device)
	{
        auto vertexDataSize = m_vertices.size() * sizeof(Vertex);
        if (!m_indices.empty())
        {
            // The mesh supports indices
            auto indexDataSize = m_indices.size() * sizeof(m_indices[0]);

            // stagingSize should be equal to vertexDataSize most of the time
            auto stagingSize = std::max(vertexDataSize, indexDataSize);
            CreateStagingBuffer(device, stagingSize);
            CreateVertexBuffer(device, vertexDataSize);
            CreateIndexBuffer(device, indexDataSize);
        }
        else
        {
            // The mesh does not support indices -> vertices only representation
            CreateStagingBuffer(device, vertexDataSize);
            CreateVertexBuffer(device, vertexDataSize);
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

    void Mesh::CreateCubeMesh()
    {
        m_vertices = {
            // Front face (Z = +0.5)
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},

            // Back face (Z = -0.5)
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

            // Right face (X = +0.5)
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},

            // Left face (X = -0.5)
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},

            // Top face (Y = +0.5)
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},

            // Bottom face (Y = -0.5)
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}}
        };

        m_indices = {
            0, 1, 2, 2, 3, 0,       // Front
            4, 5, 6, 6, 7, 4,       // Back
            8, 9, 10, 10, 11, 8,    // Left
            12, 13, 14, 14, 15, 12, // Right
            16, 17, 18, 18, 19, 16, // Top
            20, 21, 22, 22, 23, 20  // Bottom
        };
    }

    void Mesh::CreateSphereMesh(uint32_t nSlices, uint32_t nStacks) 
    {
        m_vertices.clear();
        m_indices.clear();

        constexpr float PI = 3.14159265359f;

        // One quad of the sphere mesh is addressed by a couple
        // (i, j) -> i is the "row", and j the "column"
        // i = 0 is the bottom of the sphere
        // j = 0 is 0 degrees

        // Add top vertex
        m_vertices.push_back(Vertex{
            {0, 0, 1},
            {0, 0, 1}
        });

        // Generate per stack/slice vertices
        for (uint32_t i = 0; i < nStacks - 1; i++)
        {
            // PI * 1/nStacks -> vertical step
            auto phi = PI * double(i + 1) / double(nStacks);
            for (uint32_t j = 0; j < nSlices; j++)
            {
                // 2PI * 1/nSlices -> horizontal step
                // at the first iteration it is 0
                auto theta = 2.0 * PI * double(j) / double(nSlices);
                auto x = std::sin(phi) * std::cos(theta);
                auto y = std::sin(phi) * std::sin(theta);
                auto z = std::cos(phi);
                m_vertices.push_back(Vertex{
                    {x, y, z},
                    glm::normalize(glm::vec3{x, y, z})
                });
            }
        }

        // Add bottom vertex
        m_vertices.push_back(Vertex{
            {0, 0, -1},
            {0, 0, -1}
        });

        // Add top indices
        for (uint32_t i = 0; i < nSlices; i++)
        {
            // Next vertex index computed with wraparound
            uint32_t next = (i + 1) % nSlices;

            // NOTE: CCW order
            m_indices.push_back(0); // top vertex
            m_indices.push_back(1 + i); // current index (excluding the top one)
            m_indices.push_back(1 + next); // next to current index
        }

        // Add mid section indices
        for (uint32_t i = 0; i < nStacks - 2; i++)
        {
            uint32_t currRing = 1 + i * nSlices;
            uint32_t nextRing = currRing + nSlices;

            for (uint32_t j = 0; j < nSlices; j++)
            {
                uint32_t curr = currRing + j;
                uint32_t currNext = currRing + (j + 1) % nSlices;
                uint32_t next = nextRing + j;
                uint32_t nextNext = nextRing + (j + 1) % nSlices;

                // First triangle
                m_indices.push_back(curr);     // top-left
                m_indices.push_back(next);     // bottom-left
                m_indices.push_back(currNext); // top-right

                // Second triangle
                m_indices.push_back(currNext); // top-right
                m_indices.push_back(next);     // bottom-left
                m_indices.push_back(nextNext); // bottom-right
            }
        }

        // Add bottom indices
        uint32_t bottomIndex = static_cast<uint32_t>(m_vertices.size() - 1);
        uint32_t lastRing = 1 + (nStacks - 2) * nSlices;
        for (uint32_t i = 0; i < nSlices; i++)
        {
            // Next vertex index computed with wraparound
            uint32_t next = lastRing + (i+1) % nSlices;
            // NOTE: CCW order
            m_indices.push_back(bottomIndex); // bottom vertex
            m_indices.push_back(next); // next to current index
            m_indices.push_back(lastRing + i); // current index
        }
    }

    void Mesh::CreateStagingBuffer(Device& device, vk::DeviceSize size)
    {        
        VkBufferCreateInfo stagingInfo{};
        stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingInfo.size = size;
        stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo{};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        m_stagingBuffer = std::make_unique<Buffer>(device.GetAllocator(), stagingInfo, stagingAllocInfo);
    }

    void Mesh::DestroyStagingBuffer()
    {
        m_stagingBuffer.reset();
    }

    void Mesh::CreateVertexBuffer(Device& device, vk::DeviceSize size)
    {
        VkBufferCreateInfo vertexInfo{};
        vertexInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexInfo.size = size;
        vertexInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        vertexInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo vertexAllocInfo{};
        vertexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        m_vertexBuffer = std::make_unique<Buffer>(device.GetAllocator(), vertexInfo, vertexAllocInfo);

        // Load vertex data on the staging buffer
        m_stagingBuffer->LoadData(m_vertices.data(), size);

        // Issue request to copy vertex data loaded on the staging buffer to GPU memory
        device.CopyBuffer(*m_stagingBuffer, *m_vertexBuffer, size);
    }

    void Mesh::CreateIndexBuffer(Device& device, vk::DeviceSize size)
    {
        VkBufferCreateInfo indexInfo{};
        indexInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexInfo.size = size;
        indexInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        indexInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo indexAllocInfo{};
        indexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        m_indexBuffer = std::make_unique<Buffer>(device.GetAllocator(), indexInfo, indexAllocInfo);

        // Load index data on the staging buffer
        m_stagingBuffer->LoadData(m_indices.data(), size);

        // Issue request to copy index data loaded on the staging buffer to GPU memory
        device.CopyBuffer(*m_stagingBuffer, *m_indexBuffer, size);
    }
}