//
// Created by stefano on 04/08/26.
//

#include "scene/camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/quaternion.hpp>

Camera::Camera() {
    transform.translate(glm::vec3(0.0f, 0.0f, 3.0f));
}

void Camera::moveCamera(const CameraMovement movementType, float deltaTime) {
    const float frameCameraSpeed = cameraSpeed * deltaTime;
    if (movementType == FORWARD)
        transform.translate(frameCameraSpeed * front);
    if (movementType == BACKWARD)
        transform.translate(-frameCameraSpeed * front);
    if (movementType == LEFT)
        transform.translate(-glm::normalize(glm::cross(front, up)) * frameCameraSpeed);
    if (movementType == RIGHT)
        transform.translate(glm::normalize(glm::cross(front, up)) * frameCameraSpeed);
}

void Camera::rotateCamera(float deltaX, float deltaY) {
    deltaX *= sensitivity;
    deltaY *= sensitivity;

    eulerAngles.x -= deltaY;
    eulerAngles.y -= deltaX;

    /*
        Clamp the rotation around the x axis between -89 and 89.
        Wrap the rotation around the y axis between -180 and 180.
        GLM module function maps between [0, -2pi] so we first need to add pi and than we subtract it again at the end
    */
    constexpr float limitX = glm::radians(89.0f);
    eulerAngles.x = glm::clamp(eulerAngles.x, -limitX, limitX);
    eulerAngles.y = glm::mod(eulerAngles.y + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();

    transform.rotateEuler(eulerAngles);

    front = transform.getRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
}

void Camera::updateMatrices(float aspectW, float aspectH) {
    aspectRatio = aspectW / aspectH;
    view = glm::lookAt(transform.getPosition(), transform.getPosition() + front, up);
    projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
