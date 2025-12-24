#include "GltfLoader.hpp"

#include "tiny_gltf.h"

#include "Scene.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "Common.hpp"

namespace Felina
{
	// Load all meshes in `model` and fill `meshes` with the corresponding MeshIDs
	static void LoadMeshes(tinygltf::Model& model, Renderer& renderer, std::unordered_map<int, MeshID>& meshes)
	{
		auto& rm = ResourceManager::GetInstance();
		
		// Iterate through all meshes and load them
		for (size_t i = 0; i < model.meshes.size(); i++)
		{
			auto& mesh = model.meshes[i];
			for (auto& primitive : mesh.primitives)
			{
				// NOTE: only mode currently supported is TRIANGLE_LIST (see PipelineBuilder.cpp)
				if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
					throw std::runtime_error("[GltfLoader] Unsupported mode required!");

				// Loading vertices
				std::vector<Vertex> vertices;
				{
					// Vertex position
					auto& posAccessor = model.accessors[primitive.attributes["POSITION"]];
					auto& posBufferView = model.bufferViews[posAccessor.bufferView];
					auto& posBuffer = model.buffers[posBufferView.buffer];
					// Vertex normal
					auto& normAccessor = model.accessors[primitive.attributes["NORMAL"]];
					auto& normBufferView = model.bufferViews[normAccessor.bufferView];
					auto& normBuffer = model.buffers[normBufferView.buffer];
					// Vertex UVs
					auto& uvAccessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
					auto& uvBufferView = model.bufferViews[uvAccessor.bufferView];
					auto& uvBuffer = model.buffers[uvBufferView.buffer];

					// The following assertion SHOULD be guaranteed by the implementation 
					// of the glTF-2.0 specs, so these checks are just for safety
					assert(posAccessor.type == TINYGLTF_TYPE_VEC3 && "[GltfLoader] Unexpected type found for vertex position!");
					assert(posAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && "[GltfLoader] Unexpected componentType found for vertex position!");
					assert(normAccessor.type == TINYGLTF_TYPE_VEC3 && "[GltfLoader] Unexpected type found for vertex normal!");
					assert(normAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && "[GltfLoader] Unexpected componentType found for vertex normal!");
					assert(uvAccessor.type == TINYGLTF_TYPE_VEC2 && "[GltfLoader] Unexpected type found for uv!");
					// TODO: add unsigned byte and unsigned short component type support
					assert(uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && "[GltfLoader] Unexpected componentType found for vertex normal!");

					// Reading data from the buffer
					const uint8_t* posStart = posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset;
					const uint8_t* normStart = normBuffer.data.data() + normBufferView.byteOffset + normAccessor.byteOffset;
					const uint8_t* uvStart = uvBuffer.data.data() + uvBufferView.byteOffset + uvAccessor.byteOffset;
					size_t posStride = posAccessor.ByteStride(posBufferView);
					size_t normStride = normAccessor.ByteStride(normBufferView);
					size_t uvStride = uvAccessor.ByteStride(uvBufferView);

					// The following assertion SHOULD be guaranteed by the implementation as well
					assert(posAccessor.count == normAccessor.count && "[GltfLoader] Number of vertex positions and normals differ!");
					assert(posAccessor.count == uvAccessor.count && "[GltfLoader] Number of vertex positions and uv differ!");
					
					vertices.resize(posAccessor.count);
					for (size_t i = 0; i < posAccessor.count; i++)
					{
						const float* posPtr = reinterpret_cast<const float*>(posStart + i * posStride);
						const float* normPtr = reinterpret_cast<const float*>(normStart + i * normStride);
						const float* uvPtr = reinterpret_cast<const float*>(uvStart + i * uvStride);

						vertices[i].pos = glm::vec3(posPtr[0], posPtr[1], posPtr[2]);
						vertices[i].normal = glm::vec3(normPtr[0], normPtr[1], normPtr[2]);
						vertices[i].uv = glm::vec2(uvPtr[0], uvPtr[1]);
					}
				}

				// TODO: properly handle loading meshes without indices, by calling the correct draw call
				assert(primitive.indices != -1 && "[GltfLoader] Indices are not the defined!");

				// Loading indices
				std::vector<uint32_t> indices;
				{
					auto& accessor = model.accessors[primitive.indices];
					auto& bufferView = model.bufferViews[accessor.bufferView];
					auto& buffer = model.buffers[bufferView.buffer];
					indices.resize(accessor.count);

					assert(accessor.type == TINYGLTF_TYPE_SCALAR && "[GltfLoader] Unexpected type found for indices!");
					assert(bufferView.byteStride == 0 && "[GltfLoader] Indices are not tightly packed!");
					
					// Infer the correct component type and read data
					switch (accessor.componentType)
					{
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
						{
							const uint8_t* raw = reinterpret_cast<const uint8_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
							for (size_t i = 0; i < indices.size(); i++)
								indices[i] = static_cast<uint32_t>(raw[i]);
							break;
						}

						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						{
							const uint16_t* raw = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
							for (size_t i = 0; i < indices.size(); i++)
								indices[i] = static_cast<uint32_t>(raw[i]);
							break;
						}

						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
						{
							const uint32_t* raw = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
							for (size_t i = 0; i < indices.size(); i++)
								indices[i] = raw[i];
							break;
						}
						default:
							throw std::runtime_error("[GltfLoader] Unsupported index componentType!");
					}
				}

				// Create and load mesh
				MeshID id = rm.LoadMesh(std::make_unique<Mesh>(vertices, indices), mesh.name, renderer);
				meshes.insert(std::pair<int, MeshID>(static_cast<int>(i), id));
			}
		}
		LOG("[GltfLoader] Loaded " + std::to_string(meshes.size()) + " meshes");
	}

	// Load all textures in `model` and fill `textures` with the corresponding TextureIDs
	static void LoadTextures(tinygltf::Model& model, Renderer& renderer, std::unordered_map<int, TextureID>& textures)
	{
		auto& rm = ResourceManager::GetInstance();
		for (size_t i = 0; i < model.textures.size(); i++)
		{
			tinygltf::Texture& texture = model.textures[i];
			// texture.sampler <- currently ignored
			if (texture.source == -1)
				continue;

			tinygltf::Image& image = model.images[texture.source];

			// Check if image data wasn't loaded for some reasons
			// e.g. forget to put textures in the same path as the .glTF
			if (image.image.size() == 0)
				throw std::runtime_error("[GltfLoader] Image data missing! Check previous warnings or errors from the loader.");

			// Create texture object
			vk::ImageCreateInfo imageInfo {				
				.imageType = vk::ImageType::e2D,
				.format = vk::Format::eR8G8B8A8Srgb,
				.extent = vk::Extent3D{ static_cast<uint32_t>(image.width),static_cast<uint32_t>(image.height), 1 },
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = vk::SampleCountFlagBits::e1,
				.tiling = vk::ImageTiling::eOptimal,
				.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
				.sharingMode = vk::SharingMode::eExclusive,
				.initialLayout = vk::ImageLayout::eUndefined
			};
			VmaAllocationCreateInfo allocInfo = { .usage = VMA_MEMORY_USAGE_AUTO };
			std::unique_ptr<Texture> tex = std::make_unique<Texture>(renderer.GetDevice(), imageInfo, allocInfo);

			// Load texture
			TextureID id = rm.LoadTexture(std::move(tex), texture.name, image.image.data(), image.image.size(), renderer);
			textures.insert(std::pair<int, TextureID>(static_cast<int>(i), id));
		}
		LOG("[GltfLoader] Loaded " + std::to_string(textures.size()) + " textures");
	}

	// Load all materials in `model` and fill `materials` with the corresponding MaterialIDs
	static void LoadMaterials(tinygltf::Model& model, std::unordered_map<int, TextureID>& textures, std::unordered_map<int, MaterialID>& materials)
	{
		auto& rm = ResourceManager::GetInstance();
		for (size_t i = 0; i < model.materials.size(); i++)
		{
			const auto& material = model.materials[i];

			// TODO: improve material system to PBR
			// NOTES on `baseColor`:
			// metal -> f0
			// dielectric -> albedo and f0 should be set to 0.04
			//               as a good approximation of dielectrics
			//               (see Real Time Rendering)
			auto baseColor = material.pbrMetallicRoughness.baseColorFactor;
			auto metalness = material.pbrMetallicRoughness.metallicFactor;
			auto roughness = material.pbrMetallicRoughness.roughnessFactor;
			float ambient = 0.02f;

			// According to the specs, baseColorTexture should be multiplied with baseColorFactor
			// Since Blender seems to export either one or the other this is left as TODO
			int albedoTexIndex = material.pbrMetallicRoughness.baseColorTexture.index;

			std::unique_ptr<Material> mat = std::make_unique<Material>(
				glm::vec3(baseColor[0], baseColor[1], baseColor[2]),
				glm::vec4(roughness, metalness, ambient, 0.0),
				(albedoTexIndex == -1) ? -1 : textures[albedoTexIndex]
			);
			
			MaterialID id = rm.LoadMaterial(std::move(mat), material.name);
			materials.insert(std::pair<int, MaterialID>(static_cast<int>(i), id));
		}
		LOG("[GltfLoader] Loaded " + std::to_string(materials.size()) + " materials");
	}

	// Create object and iterate recursively through its children
	static std::unique_ptr<Object> LoadNode(
		const tinygltf::Node& node, Object* parent,
		const tinygltf::Model& model, 
		const std::unordered_map<int, MeshID>& meshes, const std::unordered_map<int, MaterialID>& materials
	)
	{
		if (node.mesh == -1)
			throw std::runtime_error("[GltfLoader] " + node.name + " has no mesh");

		// Retrieve resource IDs
		MeshID meshId = meshes.at(node.mesh);
		const tinygltf::Mesh& mesh = model.meshes[node.mesh];

		// Here it is assumed that if a mesh has multiple primitives,
		// they'll be all rendered with the material used by the first one
		// TODO: add submeshes support
		MaterialID materialId = materials.at(mesh.primitives[0].material);

		// Object creation
		std::unique_ptr<Object> obj = std::make_unique<Object>(node.name, meshId, materialId, parent);

		// Apply transform
		// Transform -> see p.18 of glTF specs
		if (node.matrix.size() == 16) // if matrix is specified it will have priority
		{
			glm::mat4 mat;
			for (int i = 0; i < 16; i++)
				mat[i / 4][i % 4] = static_cast<float>(node.matrix[i]);
			obj->SetModelMatrix(mat);
		}
		else
		{
			if (node.scale.size())
				obj->SetScale(glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
			if (node.rotation.size()) // XYZW
				obj->SetRotation(glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2])); // WXYZ
			if (node.translation.size())
				obj->SetPosition(glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
		}
			
		// Iterate over its children
		for (const auto childIdx : node.children)
		{
			std::unique_ptr<Object> child = LoadNode(model.nodes[childIdx], obj.get(), model, meshes, materials);
			obj->AddChild(std::move(child)); // Move child object ownership to the parent
		}
		return std::move(obj);
	}

	// Parse filepath (either .glb or .gltf file) into model using tinygltf
	static void ParseFile(const std::filesystem::path& filepath, tinygltf::Model& model)
	{
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;
		const std::string extension = filepath.extension().string();

		// Check file extension and load file
		bool ret{};
		if (extension == ".glb")
			ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath.string());
		else if (extension == ".gltf")
			ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath.string());
		else
			throw std::runtime_error(
				"[GltfLoader] Tried to load unsupported file format.\n \
				 Currently supported file formats: .glb, .gltf."
			);

		if (!warn.empty())
			LOG("[GltfLoader] Warn: " + warn);
		if (!err.empty())
			throw std::runtime_error("[GltfLoader] Err: " + err);
		if (!ret)
			throw std::runtime_error("[GltfLoader] Failed to parse glTF: " + filepath.string());

		LOG("[GltfLoader] Parsed " + filepath.string());
	}

	// Load resources and setup `scene` with the data provided by the file in `filepath`
	// NOTE: 
	//    - `renderer` is used to call backend functions for loading resources
	//    - `filepath` must be a valid path to either a .gltf or .glb file, 
	//       otherwise an exception will be raised
	void LoadSceneFromGlTF(const std::filesystem::path& filepath, Scene& scene, Renderer& renderer)
	{
		// File parsing
		tinygltf::Model model;
		ParseFile(filepath, model);

		// Load resources
		// NOTE: texture MUST be loaded before materials!
		std::unordered_map<int, MeshID>	meshes; // Look-up between glTF indices and ResourceID
		std::unordered_map<int, TextureID> textures;
		std::unordered_map<int, MaterialID> materials;
		LoadMeshes(model, renderer, meshes);
		LoadTextures(model, renderer, textures);
		LoadMaterials(model, textures, materials);

		// Iterate through each top-level node (parent = nullptr)
		for (const auto nodeIdx : model.scenes[model.defaultScene].nodes)
		{
			std::unique_ptr<Object> obj = LoadNode(model.nodes[nodeIdx], nullptr, model, meshes, materials);
			scene.AddObject(std::move(obj)); // Move top-level object ownership to the scene
		}
	}
}