#pragma once

#include "scene/camera.h"
#include "scene/nodeWorld.h"
#include "textureStorage.h"
#include "material.h"
#include "MeshContainer.h"

struct Scene {
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<GPUMaterial> gpuMaterials;
    MeshContainer meshContainer;
    NodeWorld nodeWorld;
    
    Camera camera;
};
