//
// Created by stefano on 27/07/26.
//

#include "window.h"

#include <stdexcept>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>

Window::Window(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
}

Window::~Window() {
    destroyWindow();
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

    m_oldKeyboardState.resize(SDL_SCANCODE_COUNT, false);
}

bool Window::pollEvents() {
    SDL_Event event{0};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                return false;
            case SDL_EVENT_WINDOW_RESIZED:
                m_width = event.window.data1;
                m_height = event.window.data2;
                break;
            case SDL_EVENT_MOUSE_MOTION:
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
    }
    SDL_Quit();
}

void Window::updateInputState() {
    for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
        m_oldKeyboardState[i] = m_keyboardState[i];
    }

    m_oldMousePosition = m_mousePosition;
}
