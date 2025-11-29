#include "Material.hpp"

namespace Felina
{
	Material::Material(glm::vec3 albedo, glm::vec4 specular)
		: m_Albedo(albedo), m_Specular(specular)
	{
	}
}