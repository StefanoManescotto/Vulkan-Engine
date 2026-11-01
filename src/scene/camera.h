//
// Created by stefano on 04/08/26.
//

#pragma once

#include "scene/transform.h"
#include <fmt/base.h>

struct Plane {
    glm::vec3 normal {0.0f, 1.f, 0.0f};
    float distance = 0.0f;

    void normalize() {
        float length = glm::length(normal);
        normal /= length;
        distance /= length;
    }

    float getDistance(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

struct Frustum {
    Plane planes[6];

    void update(const glm::mat4& vp) {
        // Left Plane
        planes[0].normal = { vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0] };
        planes[0].distance = vp[3][3] + vp[3][0];

        // Right Plane
        planes[1].normal = { vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0] };
        planes[1].distance = vp[3][3] - vp[3][0];

        // Bottom Plane
        planes[2].normal = { vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1] };
        planes[2].distance = vp[3][3] + vp[3][1];

        // Top Plane
        planes[3].normal = { vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1] };
        planes[3].distance = vp[3][3] - vp[3][1];

        // Near Plane
        planes[4].normal = {vp[0][2],vp[1][2],vp[2][2] };
        planes[4].distance = vp[3][2];

        // Far Plane
        planes[5].normal = { vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2] };
        planes[5].distance = vp[3][3] - vp[3][2];
        
        for(Plane& plane : planes) {
            plane.normalize();
        }

        for (int i = 0; i < 6; ++i) {
            const Plane& p = planes[i];

            // fmt::print(
            //     "Plane {}: normal=({}, {}, {}), distance={}, length={}\n",
            //     i,
            //     p.normal.x, p.normal.y, p.normal.z,
            //     p.distance,
            //     glm::length(p.normal)
            // );
        }
    }
};

class Camera {
public:
    enum CameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
    };

    Camera();

    Transform transform;

    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float aspectRatio = 0.0f;

    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);;
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    void moveCamera(CameraMovement movement, float deltaTime);
    void rotateCamera(float deltaX, float deltaY);
    void updateMatrices(float aspectW, float aspectH);
    Frustum getFrustum() { return m_frustum; }

private:
    float cameraSpeed = 2.5f;
    float sensitivity = .01f;
    Frustum m_frustum;

    glm ::vec3 eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
};
