#pragma once

#include <vector>
#include <string>
#include <iostream>

#define PRINT(x) std::cout << x << ' '
#define PRINTLN(x) std::cout << x << '\n'

namespace Felina
{
	using MeshID = uint32_t;
	using MaterialID = uint32_t;
	using TextureID = uint32_t;

	// NOTE: originally designed to read SPIR-V file, so it
	// may need adjustments reading other file formats is required
	std::vector<char> ReadFile(const std::string& filepath);
}