#include "Material.hpp"

namespace Felina
{
	Material::Material(glm::vec3 albedo, glm::vec3 specular, glm::vec4 coefficients)
		: m_albedo(albedo), m_specular(specular), m_coefficients(coefficients)
	{
	}
}