#pragma once


#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include "Camera.hpp"

#include <vector>
#include <array>

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

	class Scene
	{
	public:
		Scene(float viewportWidth, float viewportHeigh);

		const std::vector<Vertex>& GetVertices() const { return m_vertices; }
		const std::vector<uint16_t>& GetIndices() const { return m_indices; }
		Camera& GetCamera() { return m_camera; }
		const Camera& GetCamera() const { return m_camera; }
	private:
		std::vector<Vertex> m_vertices =
		{
			{{-0.5f, -0.5f,  0.0f}, {1.0f, 1.0f, 1.0f}},
			{{ 0.5f, -0.5f,  0.0f}, {0.0f, 1.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f,  0.5f,  0.0f}, {0.0f, 0.0f, 1.0f}}
		};
		std::vector<uint16_t> m_indices = { // ! Change index type in the binding if modified
			0, 1, 2, 2, 3, 0
		};
		Camera m_camera;
	};
}