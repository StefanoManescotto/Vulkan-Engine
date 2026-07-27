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

VkShaderModule Shader::getVertexShader() const {
    return vertShader;
}

VkShaderModule Shader::getFragmentShader() const {
    return fragShader;
}

VkShaderModule Shader::createShaderModule(const std::string &fileName, shaderc_shader_kind kind, VkDevice device) const {
    // read shader file from disk
    const std::string shaderPath = SHADER_DIR + fileName;
    const std::string src = readTextFile(shaderPath);
    if (src.empty()) {
        throw std::runtime_error("Specified shader file doesn't exist: " + shaderPath);
    }

    // compile the shader to SPIR-V
    // fmt::print("Compiling shader: {}\n", shaderPath);
    // fflush(stdout);

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

    VkShaderModuleCreateInfo moduleCreateInfo
    {
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

void Shader::createShaders(VkDevice device) {
    if (vertShader = createShaderModule("shader.vert", shaderc_vertex_shader, device); !vertShader) {
        throw std::runtime_error("Error creating vertex shader module");
    }
    if (fragShader = createShaderModule("shader.frag", shaderc_fragment_shader, device); !fragShader) {
        throw std::runtime_error("Error creating fragment shader module");
    }
}
