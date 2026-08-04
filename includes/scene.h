//
// Created by stefano on 31/07/26.
//

#pragma once

#include <vector>

#include "gameObject.h"
#include "camera.h"

class Scene {
public:
    void init(VkDevice device, VmaAllocator allocator);

    Camera camera;

    GameObject cube;
    std::vector<GameObject> objects;

    void destroyScene();
private:

};
