#include "Common.hpp"

#include "Scene.hpp"
#include "Renderer.hpp"

#include <tiny_gltf.h>

#include <fstream>

namespace Felina
{
    std::vector<char> ReadFile(const std::string& filepath)
    {
        // Read starting from the end to determine the size of the file
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filepath);
        }

        std::vector<char> buffer(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        file.close();

        return buffer;
    }

	static void PrintObject(const std::unique_ptr<Object>& object, uint16_t depth)
	{
		for (size_t i = 0; i < depth; i++)
			PRINT('\t');
		PRINTLN(object->GetName());
		for (const auto& child : object->GetChildren())
			PrintObject(child, depth + 1);
	}

	void PrintSceneHierarchy(const Scene& scene)
	{
		for (const auto& obj : scene.GetObjects())
			PrintObject(obj, 0);
	}
}