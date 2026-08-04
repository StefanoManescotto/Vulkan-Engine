//
// Created by stefano on 31/07/26.
//

#include "scene.h"

void Scene::init(VkDevice device, VmaAllocator allocator) {
    cube.init(device, allocator);
}

void Scene::destroyScene() {
    cube.destroy();
}
