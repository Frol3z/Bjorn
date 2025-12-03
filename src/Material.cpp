#include "Material.hpp"

namespace Felina
{
	Material::Material(glm::vec3 albedo, glm::vec4 specular)
		: m_albedo(albedo), m_specular(specular)
	{
	}
}