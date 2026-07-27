//
// Created by stefano on 27/07/26.
//

#include "shader.h"

#include <fstream>
#include <sstream>

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

VkShaderModule Shader::createShaderModule(const std::string &fileName, shaderc_shader_kind kind, VkDevice device) const {
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
    if (vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Error creating shader module");
    }

    return shaderModule;
}

Shader::~Shader() {
    destroyShaders();
}

void Shader::createShaders(VkDevice device) {
    this->m_device = device;
    if (m_vertShader = createShaderModule("shader.vert", shaderc_vertex_shader, device); !m_vertShader) {
        throw std::runtime_error("Error creating vertex shader module");
    }
    if (m_fragShader = createShaderModule("shader.frag", shaderc_fragment_shader, device); !m_fragShader) {
        throw std::runtime_error("Error creating fragment shader module");
    }
}

void Shader::destroyShaders() {
    if (m_vertShader) {
        vkDestroyShaderModule(m_device, m_vertShader, nullptr);
        m_vertShader = nullptr;
    }
    if (m_fragShader) {
        vkDestroyShaderModule(m_device, m_fragShader, nullptr);
        m_fragShader = nullptr;
    }
}
