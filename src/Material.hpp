#pragma once

#include "Common.hpp"

#include <glm/glm.hpp>

namespace Felina
{
	class Material
	{
		public:
			Material(glm::vec3 baseColor, glm::vec4 coefficients, TextureID baseColorTex = -1);

			const glm::vec3 GetBaseColor() const { return m_baseColor; }
			const glm::vec4 GetCoefficients() const { return m_coefficients; }
			const TextureID GetBaseColorTex() const { return m_baseColorTex; }

		private:
			glm::vec3 m_baseColor;
			glm::vec4 m_coefficients; // R: roughness, G: metalness, B: ambient, A: unused
			
			// Invalid if equal to -1 unsigned int (424967...)
			TextureID m_baseColorTex;
	};
}