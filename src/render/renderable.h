//
// Created by stefano on 11/09/26.
//

#pragma once
#include <vector>

#include "GPUMesh.h"
#include "scene/transform.h"

using MeshIndex = uint32_t;
using TransformIndex = uint32_t;

struct Renderable {
    MeshIndex mesheIdx;
    TransformIndex transformIdx;
    // TODO: add material and sort by it
};

struct RenderWorld {
    std::vector<GPUMesh> gpuMeshes;
    std::vector<Transform> transforms;

    std::vector<Renderable> renderables;
};
