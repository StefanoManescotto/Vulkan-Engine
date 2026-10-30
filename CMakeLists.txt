cmake_minimum_required(VERSION 4.2)
project(VulkanEngine)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED COMPONENTS volk shaderc_combined) # shaderc_combined -> runtime shader compiler
find_package(SDL3 REQUIRED)
find_package(glm REQUIRED)
find_package(fmt REQUIRED)

add_compile_definitions(VK_NO_PROTOTYPES)

# set(SOURCES
#     src/application.cpp
#     src/main.cpp
#     src/render/mainRenderSystem.cpp
#     src/render/renderer.cpp
#     src/render/shaderManager.cpp
#     src/scene/camera.cpp
#     src/scene/materialSystem.cpp
#     src/scene/meshSystem.cpp
#     src/scene/nodeWorld.cpp
#     src/scene/scene.cpp
#     src/scene/sceneSystem.cpp
#     src/scene/textureSystem.cpp
#     src/scene/transformSystem.cpp
#     src/vk/device.cpp
#     src/vk/GPUImageSystem.cpp
#     src/vk/commandContext.cpp
#     src/vk/pipeline.cpp
#     src/vk/swapchain.cpp
#     src/window.cpp
# )

# set(HEADERS
# 	src/application.h
# 	src/render/GPUMesh.h
# 	src/render/mainRenderSystem.h
# 	src/render/renderSystem.h
# 	src/render/renderable.h
# 	src/render/renderer.h
# 	src/render/shaderManager.h
# 	src/scene/MeshContainer.h
# 	src/scene/camera.h
# 	src/scene/gameObject.h
# 	src/scene/image.h
# 	src/scene/material.h
# 	src/scene/materialSystem.h
# 	src/scene/meshSystem.h
# 	src/scene/nodeWorld.h
# 	src/scene/scene.h
# 	src/scene/sceneSystem.h
# 	src/scene/textureStorage.h
# 	src/scene/textureSystem.h
# 	src/scene/transform.h
# 	src/scene/transformSystem.h
# 	src/vk/device.h
# 	src/vk/GPUImage.h
# 	src/vk/GPUImageSystem.h
# 	src/vk/VulkanContext.h
# 	src/vk/commandContext.h
# 	src/vk/pipeline.h
# 	src/vk/swapchain.h
# 	src/window.h
# )

# add_executable(VulkanEngine ${SOURCES} ${HEADERS})

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes/imgui)

set(TEST_TINY
	includes/tinygltf.cpp
)

file(GLOB_RECURSE ALL_SOURCES 
        CONFIGURE_DEPENDS 
        src/*.cpp 
	src/*.h
)
add_executable(VulkanEngine 
        ${ALL_SOURCES} 
        ${TEST_TINY}

        # IMGUI
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/imgui_impl_sdl3.cpp
)

target_include_directories(VulkanEngine SYSTEM PRIVATE includes)
target_include_directories(VulkanEngine PRIVATE src)

target_link_libraries(VulkanEngine PRIVATE
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

target_compile_definitions(${PROJECT_NAME} PRIVATE
        RESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources/"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        SCENES_DIR="${CMAKE_SOURCE_DIR}/scenes/"
)
