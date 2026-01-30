#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <filesystem>

#define LOG(x) std::cout << x << '\n'

namespace Felina
{
	using MeshID = uint32_t;
	using MaterialID = uint32_t;
	using TextureID = uint32_t;

	const std::filesystem::path DEFAULT_SCENE{ "./assets/complex_hierarchy.glb" };
	const std::filesystem::path SKYBOX_DIR{ "./assets/skybox/" };
	const std::filesystem::path ASSETS_DIR{ "./assets/" };

	// NOTE: originally designed to read SPIR-V file, so it
	// may need adjustments reading other file formats is required
	std::vector<char> ReadFile(const std::string& filepath);
}