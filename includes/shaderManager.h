//
// Created by stefano on 30/07/26.
//

#pragma once

#include <string>
#include <vector>
#include <volk.h>
#include <shaderc/shaderc.h>


struct Shader {
    std::string name;
    VkShaderModule module;
};

class ShaderManager {
public:
    void init(VkDevice device);
    void destroyShaders();

    void addShader(std::string fileName, shaderc_shader_kind kind, std::string shadername);
    VkShaderModule getShaderModule(std::string shaderName);
private:
    std::vector<Shader> m_shaderModules;
    VkDevice m_device;
};
