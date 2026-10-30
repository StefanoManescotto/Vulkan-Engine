#include "scene/cameraController.h"

void CameraController::updateRotation(Window& window, Camera& camera) {
    if(window.isMouseButtonDown(SDL_BUTTON_LEFT) && window.getMouseDelta() != glm::vec2{0}){
        camera.rotateCamera(window.getMouseDelta().x, window.getMouseDelta().y);
        if(!m_isDragging) {
            window.hideCursor();
            m_isDragging = true;
            SDL_GetMouseState(&m_anchorX, &m_anchorY);
        }
        window.warpCursor(m_anchorX, m_anchorY);
    }
    if(window.isMouseButtonUp(SDL_BUTTON_LEFT) && m_isDragging){
        window.warpCursor(m_anchorX, m_anchorY);
        window.showCursor();
        m_isDragging = false;
    }
}

void CameraController::updateMovement(Window& window, Camera& camera, float deltaTime) {
    if (window.isKeyDown(SDL_SCANCODE_W)) {
        camera.moveCamera(camera.FORWARD, deltaTime);
    }
    if (window.isKeyDown(SDL_SCANCODE_A)) {
        camera.moveCamera(camera.LEFT, deltaTime);
    }
    if (window.isKeyDown(SDL_SCANCODE_S)) {
        camera.moveCamera(camera.BACKWARD, deltaTime);
    }
    if (window.isKeyDown(SDL_SCANCODE_D)) {
        camera.moveCamera(camera.RIGHT, deltaTime);
    }
}
