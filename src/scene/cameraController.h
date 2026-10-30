#pragma once

#include "scene/camera.h"
#include "window.h"

class CameraController {
public:
    // void update(Window& window, Camera& camera);
    void updateRotation(Window& window, Camera& camera);
    void updateMovement(Window& window, Camera& camera, float deltaTime);

private:
    bool m_isDragging = false;

    // used to keep the cursor position still when rotating the camera
    float m_anchorX = 0.0f;
    float m_anchorY = 0.0f;
};
