//
// Created by stefano on 01/08/26.
//

#include "mesh.h"

#include <cstring>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

#include "../../includes/camera.h"
#include "gameObject.h"
#include "transform.h"

Mesh::Mesh(VkDevice device, VmaAllocator allocator, Transform* transform) {
    m_device = device;
    m_allocator = allocator;
    this->transform = transform;

    createBuffers();
}

void Mesh::destroy() {
    vmaDestroyBuffer(m_allocator, vertexBuffer, vBufferAllocation);
}

void Mesh::createBuffers() {
    VkDeviceSize vBufSize{ sizeof(Vertex) * vertices.size() };
    VkDeviceSize iBufSize{ sizeof(uint16_t) * indices.size() };
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = vBufSize + iBufSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT
    };

    VmaAllocationCreateInfo vBufferAllocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo vBufferAllocInfo{};
    VkResult result = vmaCreateBuffer(m_allocator, &bufferCI, &vBufferAllocCI, &vertexBuffer, &vBufferAllocation, &vBufferAllocInfo);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer");
    }

    memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufSize);
    memcpy(static_cast<char *>(vBufferAllocInfo.pMappedData) + vBufSize, indices.data(), iBufSize);

    VkBufferDeviceAddressInfo addressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = vertexBuffer
    };

    cubeBufferAddress = vkGetBufferDeviceAddress(m_device, &addressInfo);
    indexBufferAddress = cubeBufferAddress + vBufSize;
    buffSize = vBufSize;
}

glm::mat4 Mesh::calculateModelMatrix(Camera camera) {
    // transform->rotate(glm::vec3(20,45,0));
    glm::mat4 model = transform->getModelMatrix();

    glm::mat4 mvp = camera.projection * camera.view * model;
    return mvp;
}
