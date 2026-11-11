#pragma once

#include "volk.h"
#include <string>

struct Shader {
    std::string name;
    VkShaderModule module;
    VkShaderStageFlagBits stage;

    bool operator==(Shader& other) {
        return other.module == this->module;
    }
};
