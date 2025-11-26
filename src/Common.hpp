#pragma once

#include <vector>
#include <string>

namespace Felina
{
	// NOTE: originally designed to read SPIR-V file, so it
	// may need adjustments reading other file formats is required
	std::vector<char> ReadFile(const std::string& filename);
}