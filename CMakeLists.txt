cmake_minimum_required(VERSION 4.2)
project(Vulkan_Engine)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED COMPONENTS volk shaderc_combined) # shaderc_combined -> runtime shader compiler
find_package(SDL3 REQUIRED)
find_package(glm REQUIRED)
find_package(fmt REQUIRED)

add_compile_definitions(VK_NO_PROTOTYPES)

add_executable(Vulkan_Engine src/main.cpp
        src/application.cpp
        includes/application.h
        src/render/swapchain.cpp
        includes/swapchain.h
        src/device.cpp
        includes/device.h
        src/window.cpp
        includes/window.h
        src/render/pipeline.cpp
        includes/pipeline.h
        src/render/renderer.cpp
        includes/renderer.h
        includes/renderSystem.h
        src/render/mainRenderSystem.cpp
        includes/mainRenderSystem.h
        src/Image.cpp
        includes/image.h
        src/render/shaderManager.cpp
        includes/shaderManager.h
        src/scene/scene.cpp
        includes/scene.h
        src/scene/gameObject.cpp
        includes/gameObject.h
        src/scene/mesh.cpp
        includes/mesh.h
        src/scene/camera.cpp
        includes/camera.h
        includes/transform.h
)

target_include_directories(Vulkan_Engine PRIVATE includes)

target_link_libraries(Vulkan_Engine PRIVATE
        Vulkan::Vulkan
        Vulkan::volk
        Vulkan::shaderc_combined

        glslang
        SPIRV
        SPIRV-Tools
        SPIRV-Tools-opt

        SDL3::SDL3
        glm::glm
        fmt::fmt
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders/"
)
