//
// Created by stefano on 31/07/26.
//

#pragma once  

#include <filesystem>
#include <string>
#include <tiny_gltf_v3.h>
#include <vector>

// #include "scene/gameObject.h"
#include "scene/image.h"
#include "scene/camera.h"

#include "vk/VulkanContext.h"
#include "render/renderable.h"
#include "scene/textureStorage.h"
#include "scene/material.h"
#include "scene/MeshContainer.h"

struct SceneObjects {
  std::vector<Texture> textures;
  std::vector<Material> materials;
  MeshContainer meshContainer;
};

// struct Image {
//     int width;
//     int height;
//     int channels;
//     unsigned char *data;
// };

class Scene {
public:
  void init(VkDevice device, VmaAllocator allocator);
  void loadScene(TextureStorage &textureManager);

  Camera camera;

  RenderWorld renderWorld;
  Renderable renderable;
  VulkanCtx ctx;

  // GameObject cube;
  // std::vector<GameObject> objects;

  void destroyScene();

private:
  void loadGltf(const std::string &filepath);
  std::vector<Image> loadImages(tg3_model model, std::filesystem::path path);

  // TextureManager *m_textureManager = nullptr;
};
