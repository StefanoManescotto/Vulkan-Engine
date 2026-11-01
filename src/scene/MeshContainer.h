//
// Created by stefano on 11/09/26.
//

#pragma once
#include <cfloat>
#include <fmt/base.h>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "camera.h"

struct BoundingBox {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);

    bool isBoxVisible(const Frustum& frustum) {
        for (const auto& plane : frustum.planes) {
            glm::vec3 pVertex = this->min;
            if (plane.normal.x >= 0) pVertex.x = this->max.x;
            if (plane.normal.y >= 0) pVertex.y = this->max.y;
            if (plane.normal.z >= 0) pVertex.z = this->max.z;
            float distance = plane.getDistance(pVertex);
            // fmt::print(
            // "normal=({}, {}, {}), d={}, distance={}\n",
            //     plane.normal.x,
            //     plane.normal.y,
            //     plane.normal.z,
            //     plane.distance,
            //     distance
            // );

            if (plane.getDistance(pVertex) < 0.0f) {
                return false;
            }
        }
        return true;
    }

    BoundingBox transformAABB(const glm::mat4& m) {
        glm::vec3 center = (min + max) * 0.5f;
        glm::vec3 extents = (max - min) * 0.5f;

        glm::vec3 newCenter = glm::vec3(m * glm::vec4(center, 1.0f));

        glm::mat3 absMatrix = glm::mat3(
            glm::abs(m[0]),
            glm::abs(m[1]),
            glm::abs(m[2])
        );

        glm::vec3 newExtents = absMatrix * extents;

        BoundingBox result;
        result.min = newCenter - newExtents;
        result.max = newCenter + newExtents;

        return result;
    }
};

struct Vertex {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    glm::vec3 normal = glm::vec3(0.0f);
    glm::vec2 uv = glm::vec2(0.0f);
};

struct SubMesh {
    size_t vertexStart = 0;
    size_t vertexCount = 0;
    size_t indexStart = 0;
    size_t indexCount = 0;
    uint32_t materialId = 0;
};

struct Mesh {
    std::string name;
    std::vector<SubMesh> subMeshes;
    BoundingBox boundingBox;
};

struct MeshContainer {
    std::vector<Mesh> meshes;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    size_t vertOffset = 0; // TODO: see if these needs to be here or can just be local in the loader function
    size_t idxOffset = 0;
};




// inline std::vector<Vertex> vertices = {
//     // Front face (Z = +0.5) | Normal: (0, 0, 1)
//     { {-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f} },
//     { { 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f} },
//     { { 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f} },
//     { {-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f} },
//
//     // Back face (Z = -0.5) | Normal: (0, 0, -1)
//     { { 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f} },
//     { {-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f} },
//     { {-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f} },
//     { { 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f} },
//
//     // Left face (X = -0.5) | Normal: (-1, 0, 0)
//     { {-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },
//     { {-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
//     { {-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
//     { {-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
//
//     // Right face (X = +0.5) | Normal: (1, 0, 0)
//     { { 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },
//     { { 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
//     { { 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
//     { { 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
//
//     // Top face (Y = +0.5) | Normal: (0, 1, 0)
//     { {-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f} },
//     { { 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f} },
//     { { 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f} },
//     { {-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f} },
//
//     // Bottom face (Y = -0.5) | Normal: (0, -1, 0)
//     { {-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f} },
//     { { 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f} },
//     { { 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f} },
//     { {-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f} }
// };

// -----------------------------------------------------------------------------
// Indices: 36 indices total (2 triangles / 6 indices per face)
// Winding order: Counter-Clockwise (CCW)
// -----------------------------------------------------------------------------
// inline std::vector<uint32_t> indices = {
//      0,  1,  2,    2,  3,  0, // Front
//      4,  5,  6,    6,  7,  4, // Back
//      8,  9, 10,   10, 11,  8, // Left
//     12, 13, 14,   14, 15, 12, // Right
//     16, 17, 18,   18, 19, 16, // Top
//     20, 21, 22,   22, 23, 20  // Bottom
// };
