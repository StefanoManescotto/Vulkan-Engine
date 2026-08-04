//
// Created by stefano on 31/07/26.
//

#pragma once

#include <glm/glm.hpp>
#include <thirdparty/vk_mem_alloc.h>

#include "transform.h"
#include "mesh.h"

// struct Transform {};

class GameObject {
public:
    void init(VkDevice device, VmaAllocator allocator);

    std::vector<Mesh> meshes;
    Transform transform;

    void destroy();
private:
    VkDevice m_device;
    VmaAllocator m_allocator;
};
