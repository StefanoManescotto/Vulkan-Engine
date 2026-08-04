//
// Created by stefano on 31/07/26.
//

#include "gameObject.h"

#include <iostream>
#include <transform.h>

void GameObject::init(VkDevice device, VmaAllocator allocator) {
    m_device = device;
    m_allocator = allocator;

    meshes.emplace_back(device, allocator, &transform);
}

void GameObject::destroy() {
    for (auto m : meshes) {
        m.destroy();
    }
}
