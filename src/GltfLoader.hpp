#pragma once

#include <string>

/*
 * Missing stuff:
 * - setup scene camera from data read from the glTF.
 * - add per primitive materials, right now it is assumed that 
 *   each mesh has only one primitive and so that primitive material is used for the entire mesh
 * - handle nodes without a mesh
 */

namespace Felina {
	class Scene;
	class Renderer;

	void LoadSceneFromGlTF(const std::string& filepath, Scene& scene, Renderer& renderer);
}