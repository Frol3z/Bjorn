#pragma once

#include <filesystem>

namespace Felina {
	class Scene;
	class Renderer;

	void LoadSceneFromGlTF(const std::filesystem::path& filepath, Scene& scene, Renderer& renderer);
}