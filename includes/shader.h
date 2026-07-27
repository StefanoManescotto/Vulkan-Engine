//
// Created by stefano on 27/07/26.
//
#pragma once

#include <string>
#include <volk.h>
#include <shaderc/shaderc.hpp>

class Shader {
public:
    Shader() = default;
    ~Shader();

    /// @brief Creates the vertex and fragment shaders module.
    /// @param device The VkDevice needed to create the shader modules.
    void createShaders(VkDevice device);
    void destroyShaders();

    [[nodiscard]] VkShaderModule getVertexShader() const { return m_vertShader; };
    [[nodiscard]] VkShaderModule getFragmentShader() const { return m_fragShader; };

private:
    VkShaderModule m_vertShader = nullptr;
    VkShaderModule m_fragShader = nullptr;

    VkDevice m_device = nullptr;

    VkShaderModule createShaderModule(const std::string &fileName, shaderc_shader_kind kind, VkDevice device) const;
};
