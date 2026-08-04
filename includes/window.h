//
// Created by stefano on 27/07/26.
//
#pragma once

#include <vector>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_vulkan.h>
#include <glm/glm.hpp>

class Window {
public:
    Window() = default;
    Window(uint32_t width, uint32_t height);
    ~Window();

    /// Creates the VkSurfaceKHR
    void createSurface(VkInstance vkInstance);

    /**
     * @brief Cycles through all window events.
     * @return false if the event is SDL_EVENT_QUIT, true otherwise.
     */
    bool pollEvents();

    [[nodiscard]] SDL_Window* handle() const { return m_window; };

    [[nodiscard]] uint32_t getWidth() const { return m_width; }

    [[nodiscard]] uint32_t getHeight() const { return m_height; }

    [[nodiscard]] VkSurfaceKHR getSurface() const { return m_surface; }

    /// Return the mouse position in the window on the current frame
    [[nodiscard]] glm::vec2 getMousePosition() const { return m_mousePosition; };

    /// Return the difference of the mouse position between frames.
    glm::vec2 getMouseDelta();

    /// Returns true continuously as long as the key is held down.
    bool isKeyDown(SDL_Scancode key);

    /// Returns true ONLY on the first frame the key was pressed.
    bool isKeyPressed(SDL_Scancode key);

    // bool isMouseButtonPressed(uint8_t mouseButton) const { mouseState.; }

    /// Creates the SDL window.
    void createWindow();

    /// Destroys the SDL window.
    void destroyWindow();

    /// Destroys the VkSurface
    void destroySurface();

    /**
     * @brief Update the old keyboard and mouse position states.
     */
    void updateInputState();

private:
    SDL_Window* m_window = nullptr;
    VkSurfaceKHR m_surface = nullptr;

    uint32_t m_width = 1280;
    uint32_t m_height = 720;

    const bool* m_keyboardState = SDL_GetKeyboardState(nullptr);
    std::vector<bool> m_oldKeyboardState;

    glm::vec2 m_mousePosition {0.0f};
    glm::vec2 m_mouseDelta {0.0f};

    VkInstance vkInstance;

    // std::unordered_map<uint8_t, bool> m_mouseButtons;
};
