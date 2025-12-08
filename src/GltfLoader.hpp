#pragma once

#include <string>

namespace Felina {
	class Scene;
	class Renderer;

	void LoadSceneFromGlTF(const std::string& filepath, Scene& scene, Renderer& renderer);
}