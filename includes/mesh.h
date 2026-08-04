//
// Created by stefano on 01/08/26.
//

#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <volk.h>
#include <thirdparty/vk_mem_alloc.h>
// #include <glm/gtc/type_aligned.hpp>

// #include "application.h"

struct Transform;
struct Camera;

struct Vertex {
    glm::vec3 pos;struct MeshPushConstants {
    uint64_t vertexBufferAddress; // Direct BDA address to vertices
    uint64_t _padding;
    // uint32_t textureIndex;
    glm::vec4 color;
    glm::mat4 modelMatrix;
};
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh {
public:
    Mesh(VkDevice device, VmaAllocator allocator, Transform* transform);

    uint64_t cubeBufferAddress;
    uint64_t indexBufferAddress;
    uint64_t buffSize;

    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;

    glm::mat4 calculateModelMatrix(Camera camera);
    void destroy();
private:


    // VkBuffer vBuffer;
    VmaAllocation vBufferAllocation;
    VkDevice m_device;
    VmaAllocator m_allocator;
    Transform* transform;

    void createBuffers();
};

inline std::vector<Vertex> vertices = {
    // Front face (Z = +0.5) | Normal: (0, 0, 1)
    { {-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f} },

    // Back face (Z = -0.5) | Normal: (0, 0, -1)
    { { 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f} },
    { {-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f} },
    { {-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f} },
    { { 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f} },

    // Left face (X = -0.5) | Normal: (-1, 0, 0)
    { {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },
    { {-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
    { {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },

    // Right face (X = +0.5) | Normal: (1, 0, 0)
    { { 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
    { { 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },

    // Top face (Y = +0.5) | Normal: (0, 1, 0)
    { {-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f} },
    { { 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f} },

    // Bottom face (Y = -0.5) | Normal: (0, -1, 0)
    { {-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f} },
    { { 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f} },
    { {-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f} }
};

// -----------------------------------------------------------------------------
// Indices: 36 indices total (2 triangles / 6 indices per face)
// Winding order: Counter-Clockwise (CCW)
// -----------------------------------------------------------------------------
inline std::vector<uint16_t> indices = {
     0,  1,  2,    2,  3,  0, // Front
     4,  5,  6,    6,  7,  4, // Back
     8,  9, 10,   10, 11,  8, // Left
    12, 13, 14,   14, 15, 12, // Right
    16, 17, 18,   18, 19, 16, // Top
    20, 21, 22,   22, 23, 20  // Bottom
};
