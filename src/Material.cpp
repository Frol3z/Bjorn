#include "Material.hpp"

namespace Felina
{
	Material::Material(glm::vec3 albedo, glm::vec3 specular, glm::vec4 coefficients, 
					   TextureID albedoTex, TextureID specularTex
	)
		: m_albedo(albedo), m_specular(specular), m_coefficients(coefficients),
		  m_albedoTex(albedoTex), m_specularTex(specularTex)
	{
	}
}