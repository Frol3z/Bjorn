#include "Material.hpp"

namespace Felina
{
	Material::Material(glm::vec3 baseColor, glm::vec4 metallicRoughness, TextureID baseColorTex, TextureID metallicRoughnessTex)
		: m_baseColor(baseColor), m_metallicRoughness(metallicRoughness), 
		  m_baseColorTex(baseColorTex), m_metallicRoughnessTex(metallicRoughnessTex)
	{
	}
}