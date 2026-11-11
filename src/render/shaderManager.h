//
// Created by stefano on 30/07/26.
//

#pragma once

#include <string>
#include <vector>
#include <volk.h>
#include <shaderc/shaderc.h>
#include <vulkan/vulkan_core.h>

#include "shader.h"

class ShaderManager {
public:
    void init(VkDevice device);
    void destroyShaders();

    void addShader(std::string fileName, shaderc_shader_kind kind, std::string shadername);
    Shader& getShader(const std::string shaderName);
    VkShaderModule getShaderModule(const std::string shaderName);
private:
    std::vector<Shader> m_shaderModules;
    VkDevice m_device;
};
