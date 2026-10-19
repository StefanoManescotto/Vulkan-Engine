#pragma once

#include <vulkan/vk_enum_string_helper.h>
#include <fmt/printf.h>

#define VK_CHECK(x)                                                         \
    do {                                                                    \
        VkResult err = x;                                                   \
        if(err) {                                                          \
            fmt::print("Detected Vulkan error: {}", string_VkResult(err));  \
            abort();                                                        \
        }                                                                   \
    } while(0)
