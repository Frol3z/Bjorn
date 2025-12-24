#include "Material.hpp"

namespace Felina
{
	Material::Material(glm::vec3 baseColor, glm::vec4 coefficients, TextureID baseColorTex)
		: m_baseColor(baseColor), m_coefficients(coefficients), m_baseColorTex(baseColorTex)
	{
	}
}