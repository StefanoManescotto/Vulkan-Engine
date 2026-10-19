//
// Created by stefano on 31/07/26.
//

#include "scene/oldScene.h"
#include "tiny_gltf_v3.h"
#include <filesystem>

#include <fmt/printf.h>
#include <stb_image.h>

#include "scene/MeshContainer.h"
#include "scene/meshSystem.h"

// void Scene::init(VkDevice device, VmaAllocator allocator) {
//   // cube.init(device, allocator);

//   // preallocate memory for vertex and index data
//   // constexpr size_t vertexBufferSize = 64 * 1024 * 1024; // 64 MB
//   // constexpr size_t indexBufferSize = 32 * 1024 * 1024; // 32 MB
//   // constexpr size_t totalVerts = vertexBufferSize / sizeof(Vertex);
//   // constexpr size_t totalIndices = indexBufferSize / sizeof(uint32_t);
//     MeshContainer mesh;
//   // mesh.vertices = vertices;
//   // mesh.indices = indices;

//     Transform meshTransform;

//     ctx = VulkanCtx(device, allocator);
//     GPUMesh gpuMesh;
//     MeshSystem::createBuffer(ctx, gpuMesh, mesh);

//     renderWorld.gpuMeshes.push_back(gpuMesh);
//     renderWorld.transforms.push_back(meshTransform);
// }

// void Scene::loadScene(TextureStorage &textureManager) {
//   // m_textureManager = &textureManager;
//     loadGltf("/home/stefano/CLionProjects/VulkanEngine/scenes/mario_kart/scene.gltf");
// }

// void Scene::destroyScene() {
//   // cube.destroy();
//     for (GPUMesh &gpuMesh : renderWorld.gpuMeshes) {
//         MeshSystem::destroyBuffer(ctx, gpuMesh);
//         // TODO: remove from array
//     }
// }

// void Scene::loadGltf(const std::string &filepath) {
//     if (!std::filesystem::exists(filepath)) {
//         fmt::print("File doesn't exist : {}\n", filepath);
//         return;
//     }
//     fmt::print("Loading GLTF: {}\n", filepath);

//     tg3_model model;
//     tg3_parse_options opts;
//     tg3_error_stack errors;

//     tg3_parse_options_init(&opts);
//     tg3_error_stack_init(&errors);
//     tg3_error_code parseResult =
//     tg3_parse_file(&model, &errors, filepath.c_str(), filepath.size(), &opts);

//     if (parseResult != TG3_OK) {
//         fmt::print("Parsing errors:\n");
//         for (int i = 0; errors.count; i++) {
//         fmt::print("{}\n", errors.entries[i].message);
//         }
//         tg3_error_stack_free(&errors);
//         return;
//     }
//     tg3_error_stack_free(&errors);

//     std::filesystem::path imageDir =
//     std::filesystem::path(filepath).parent_path();
//     std::vector<Image> images = loadImages(model, imageDir);
// }

// std::vector<Image> Scene::loadImages(tg3_model model, std::filesystem::path path) {
//     std::vector<Image> images(model.images_count);
//     for (int i = 0; i < model.images_count; i++) {
//         Image &img = images[i];
//         std::filesystem::path imagePath =
//             path / model.images[i].uri.data; // get the absolute path
//         fmt::print("Loading image {}/{}: {}\n", i + 1, model.images_count,
//                 model.images[i].uri.data);
//         img.data = stbi_load(path.c_str(), &img.width, &img.height, &img.channels,
//                             STBI_rgb_alpha);
//         if (!img.data) {
//             fmt::print("Failed to load image {}\n", path.c_str());
//         }
//     }
//     return images;
// }
