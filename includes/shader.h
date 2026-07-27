//
// Created by stefano on 27/07/26.
//
#pragma once

#include <string>
#include <volk.h>
#include <shaderc/shaderc.hpp>

class Shader {
public:
    void createShaders(VkDevice device);

    [[nodiscard]] VkShaderModule getVertexShader() const;
    [[nodiscard]] VkShaderModule getFragmentShader() const;

private:
    VkShaderModule vertShader = nullptr;
    VkShaderModule fragShader = nullptr;

    VkShaderModule createShaderModule(const std::string &fileName, shaderc_shader_kind kind, VkDevice device) const;
};
