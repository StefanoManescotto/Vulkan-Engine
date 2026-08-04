//
// Created by stefano on 04/08/26.
//

#include "camera.h"

#include <glm/ext/matrix_clip_space.hpp>

Camera::Camera() {
    transform.position = glm::vec3(0.0f, 0.0f, 3.0f);
}

void Camera::moveCamera(const CameraMovement movementType, float deltaTime) {
    const float frameCameraSpeed = cameraSpeed * deltaTime;
    if (movementType == FORWARD)
        transform.position += frameCameraSpeed * front;
    if (movementType == BACKWARD)
        transform.position -= frameCameraSpeed * front;
    if (movementType == LEFT)
        transform.position -= glm::normalize(glm::cross(front, up)) * frameCameraSpeed;
    if (movementType == RIGHT)
        transform.position += glm::normalize(glm::cross(front, up)) * frameCameraSpeed;
}

void Camera::rotateCamera(float deltaX, float deltaY) {
    deltaX *= sensitivity;
    deltaY *= -sensitivity;

    transform.rotation.x += deltaY;
    transform.rotation.y += deltaX;

    if (transform.rotation.x  > 89.0f) {
        transform.rotation.x = 89.0f;
    }
    if (transform.rotation.x < -89.0f) {
        transform.rotation.x = -89.0f;
    }

    float yawRad   = glm::radians(transform.rotation.y - 90.0f);
    float pitchRad = glm::radians(transform.rotation.x);

    front.x = cos(yawRad) * cos(pitchRad);
    front.y = sin(pitchRad);
    front.z = sin(yawRad) * cos(pitchRad);
    front   = glm::normalize(front);
}

void Camera::updateMatrices(float aspectW, float aspectH) {
    aspectRatio = aspectW / aspectH;
    view = glm::lookAt(transform.position, transform.position + front, up);
    projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
