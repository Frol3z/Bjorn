#pragma once

#include "Common.hpp"

#include <glm/glm.hpp>

namespace Felina
{
	class Material
	{
		public:
			Material(glm::vec3 baseColor, glm::vec4 metallicRoughness, TextureID baseColorTex = -1, TextureID metallicRoughnessTex = -1);

			const glm::vec3 GetBaseColor() const { return m_baseColor; }
			const glm::vec4 GetMetallicRoughness() const { return m_metallicRoughness; }
			const TextureID GetBaseColorTexture() const { return m_baseColorTex; }
			const TextureID GetMetallicRoughnessTexture() const { return m_metallicRoughnessTex; }

		private:
			glm::vec3 m_baseColor;
			glm::vec4 m_metallicRoughness; // R: unused, G: roughness, B: metalness, A: unused
			
			// Invalid if equal to -1 unsigned int (424967...)
			TextureID m_baseColorTex;
			TextureID m_metallicRoughnessTex;
	};
}