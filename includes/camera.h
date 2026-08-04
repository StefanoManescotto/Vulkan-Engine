//
// Created by stefano on 04/08/26.
//

#pragma once

#include "transform.h"

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

private:
    float cameraSpeed = 2.5f;
    float sensitivity = .1f;
};
