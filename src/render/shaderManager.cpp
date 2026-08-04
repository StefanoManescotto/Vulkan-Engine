//
// Created by stefano on 30/07/26.
//

#include "shaderManager.h"

#include <fstream>
#include <sstream>
#include <shaderc/shaderc.hpp>

std::string readTextFile(const std::string &filePath) {
    std::ifstream infile(filePath);
    if (infile.is_open()) {
        std::stringstream buffer;
        buffer << infile.rdbuf();
        const std::string output = buffer.str();
        infile.close();
        return output;
    }
    return {};
}

void ShaderManager::init(VkDevice device) {
    m_device = device;
}

void ShaderManager::destroyShaders() {
    for (auto m : m_shaderModules) {
        if (m.module != nullptr) {
            vkDestroyShaderModule(m_device, m.module, nullptr);
            m.module = nullptr;
            m.name.clear();
        }
    }
}

void ShaderManager::addShader(std::string fileName, shaderc_shader_kind kind, std::string shadername) {
    const std::string shaderPath = SHADER_DIR + fileName;
    const std::string src = readTextFile(shaderPath);
    if (src.empty()) {
        throw std::runtime_error("Specified shader file doesn't exist: " + shaderPath);
    }

    shaderc::Compiler compiler;
    shaderc::CompileOptions opts;
    opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
    opts.SetTargetSpirv(shaderc_spirv_version_1_6);
    opts.SetOptimizationLevel(shaderc_optimization_level_performance);
    shaderc::CompilationResult result = compiler.CompileGlslToSpv(src, kind, fileName.c_str(), opts);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error("Shader Compilation Error: " + result.GetErrorMessage());
    }
    std::vector<uint32_t> spv = {result.cbegin(), result.cend()};

    VkShaderModuleCreateInfo moduleCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spv.size() * sizeof(uint32_t),
        .pCode = spv.data()
    };
    VkShaderModule shaderModule = nullptr;
    if (vkCreateShaderModule(m_device, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Error creating shader module");
    }

    Shader newShader { .name = shadername, .module = shaderModule };
    m_shaderModules.push_back(newShader);
}

VkShaderModule ShaderManager::getShaderModule(std::string shaderName) {
    for (auto m : m_shaderModules) {
        if (m.name == shaderName) {
            return m.module;
        }
    }
    return nullptr;
}
