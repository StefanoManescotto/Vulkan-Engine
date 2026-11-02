cmake_minimum_required(VERSION 3.25)
project(VulkanEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED COMPONENTS volk shaderc_combined) # shaderc_combined -> runtime shader compiler
find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes/imgui)

file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS
        src/*.cpp
        src/*.h
)

add_executable(VulkanEngine
        ${ALL_SOURCES}
        includes/tinygltf.cpp

        # ImGui
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/imgui_impl_sdl3.cpp
)

target_include_directories(VulkanEngine SYSTEM PRIVATE includes)
target_include_directories(VulkanEngine PRIVATE src)

target_compile_definitions(VulkanEngine PRIVATE
        VK_NO_PROTOTYPES
        IMGUI_IMPL_VULKAN_USE_VOLK
        SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
        RESOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
        SCENES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/scenes/"
)

if(MSVC)
        target_compile_options(VulkanEngine PRIVATE /utf-8)
        target_compile_definitions(VulkanEngine PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

target_link_libraries(VulkanEngine PRIVATE
        Vulkan::volk
        Vulkan::shaderc_combined
        SDL3::SDL3
        glm::glm
        fmt::fmt
)

if(UNIX AND NOT APPLE)
        target_link_libraries(VulkanEngine PRIVATE
                glslang
                SPIRV
                SPIRV-Tools-opt
                SPIRV-Tools
        )
endif()