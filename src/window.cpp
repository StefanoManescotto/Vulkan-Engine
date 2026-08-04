//
// Created by stefano on 27/07/26.
//

#include "window.h"

#include <stdexcept>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <volk.h>
#include <glm/vec2.hpp>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_vulkan.h>

bool firstMouse = true;

Window::Window(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
}

Window::~Window() {
    destroySurface();
    destroyWindow();
}

glm::vec2 Window::getMouseDelta() {
    return m_mouseDelta;
}

bool Window::isKeyDown(SDL_Scancode key) {
    return m_keyboardState[key];
}

bool Window::isKeyPressed(SDL_Scancode key) {
    return m_keyboardState[key] && !m_oldKeyboardState[key];
}

void Window::createWindow() {
    if (m_window != nullptr) {
        throw std::runtime_error("Window already exists");
    }

    SDL_InitSubSystem(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow("Vulkan Learning", m_width, m_height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        throw std::runtime_error("Error creating window");
    }

    SDL_SetWindowRelativeMouseMode(m_window, true);
    m_oldKeyboardState.resize(SDL_SCANCODE_COUNT, false);
}

void Window::createSurface(VkInstance vkInstance) {
    this->vkInstance = vkInstance;
    if (!SDL_Vulkan_CreateSurface(m_window, vkInstance, nullptr, &m_surface)) {
        throw std::runtime_error("Error creating surface");
    }
}

bool Window::pollEvents() {
    m_mouseDelta = glm::vec2(0.0f);

    SDL_Event event{0};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                return false;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            case SDL_EVENT_WINDOW_RESIZED:
                m_width = event.window.data1;
                m_height = event.window.data2;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                m_mouseDelta.x += event.motion.xrel;
                m_mouseDelta.y += event.motion.yrel;

                m_mousePosition.x = event.motion.x;
                m_mousePosition.y = event.motion.y;
                break;
            // case SDL_EVENT_MOUSE_BUTTON_DOWN:
            //     m_mouseButtons[event.button.button] = true;
            //     break;
            // case SDL_EVENT_MOUSE_BUTTON_UP:
            //     m_mouseButtons[event.button.button] = false;
            //     break;
        }
    }
    return true;
}

void Window::destroyWindow() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}

void Window::destroySurface() {
    if (m_surface) {
        vkDestroySurfaceKHR(vkInstance, m_surface, nullptr);
        m_surface = nullptr;
    }
}

void Window::updateInputState() {
    for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
        m_oldKeyboardState[i] = m_keyboardState[i];
    }
    // m_mouseDelta = glm::vec2(0.0f, 0.0f);
}
