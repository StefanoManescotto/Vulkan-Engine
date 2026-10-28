//
// Created by stefano on 11/09/26.
//

#include "scene/meshSystem.h"

#include <cstring>
#include <volk.h>

#include <vk_mem_alloc.h>

namespace MeshSystem {
    std::vector<uint32_t> loadMeshes(const tg3_model &model, const std::vector<uint32_t> &materialIds, MeshContainer &meshVerts) {
		std::vector<Mesh> &meshes = meshVerts.meshes;
		std::vector<uint32_t> meshIds(model.meshes_count);
		// load all gltf mesh and primitive data
		for (int i = 0; i < model.meshes_count; ++i) {
			Mesh mesh;
			const tg3_mesh *tg3mesh = &model.meshes[i];
			mesh.name = tg3mesh->name.data != nullptr ? tg3mesh->name.data : "No Name";

			// lambda to perform attribute data copy
			auto writeAttribute = [&model, &meshVerts]<typename T>(T Vertex::* member, const tg3_str_int_pair* attr) -> void {
				const tg3_accessor *accessor = &model.accessors[attr->value];
				const tg3_buffer_view *bufferView = &model.buffer_views[accessor->buffer_view];
				const tg3_buffer *buffer = &model.buffers[bufferView->buffer];
				const size_t bufferOffset = bufferView->byte_offset + accessor->byte_offset;
				const size_t stride = bufferView->byte_stride != 0 ? bufferView->byte_stride : sizeof(T);

				for (uint64_t idx = 0; idx < accessor->count; idx++) {
					const size_t elementOffset = bufferOffset + idx * stride;
					const float* data = reinterpret_cast<const float*>(buffer->data.data + elementOffset);
					if constexpr (std::is_same<T, glm::vec3>()) {
						meshVerts.vertices[meshVerts.vertOffset + idx].*member = glm::vec3(data[0], data[1], data[2]);
					}
					else if constexpr (std::is_same<T, glm::vec2>()) {
						meshVerts.vertices[meshVerts.vertOffset + idx].*member = glm::vec2(data[0], data[1]);
					}
				}
			};

			// copy the vertex data
			mesh.subMeshes.resize(tg3mesh->primitives_count);
			for (int j = 0; j < tg3mesh->primitives_count; j++) {
				const tg3_primitive *primitive = &tg3mesh->primitives[j];
				mesh.subMeshes[j].materialId = materialIds[primitive->material];
				mesh.subMeshes[j].vertexStart = meshVerts.vertOffset;


				for (int k = 0; k < primitive->attributes_count; k++) {
					const tg3_str_int_pair *attr = &primitive->attributes[k];
					if (strcmp(attr->key.data, "POSITION") == 0) {
						const tg3_accessor *accessor = &model.accessors[attr->value];
						assert(accessor->type == TG3_TYPE_VEC3 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT);
						assert(meshVerts.vertOffset + accessor->count <= meshVerts.vertices.size() && "Not enough space to load vertices");

						mesh.subMeshes[j].vertexCount = accessor->count;
						writeAttribute(&Vertex::position, attr);
					}
					else if (strcmp(attr->key.data, "NORMAL") == 0) {
						const tg3_accessor *accessor = &model.accessors[attr->value];
						assert(accessor->type == TG3_TYPE_VEC3 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT);
						writeAttribute(&Vertex::normal, attr);
					}
					else if (strcmp(attr->key.data, "COLOR_0") == 0) {
						const tg3_accessor *accessor = &model.accessors[attr->value];
						assert(accessor->type == TG3_TYPE_VEC3 || accessor->type == TG3_TYPE_VEC4);
						assert(accessor->component_type == TG3_COMPONENT_TYPE_FLOAT);
						writeAttribute(&Vertex::color, attr);
					}
					else if (strcmp(attr->key.data, "TEXCOORD_0") == 0) {
						const tg3_accessor *accessor = &model.accessors[attr->value];
						assert(accessor->type == TG3_TYPE_VEC2 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT);
						writeAttribute(&Vertex::uv, attr);
					}
				}
				meshVerts.vertOffset += mesh.subMeshes[j].vertexCount;

				// copy index data
				if (primitive->indices != -1) {
					const tg3_accessor *accessor = &model.accessors[primitive->indices];
					const tg3_buffer_view *bufferView = &model.buffer_views[accessor->buffer_view];
					const tg3_buffer *buffer = &model.buffers[bufferView->buffer];
					assert(meshVerts.idxOffset + accessor->count <= meshVerts.indices.size() && "Not enough space for indices");

					mesh.subMeshes[j].indexStart = meshVerts.idxOffset;
					mesh.subMeshes[j].indexCount = accessor->count;

					if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) {
						const uint32_t* buffData = reinterpret_cast<const uint32_t*>(buffer->data.data + bufferView->byte_offset + accessor->byte_offset);
						memcpy(&meshVerts.indices[meshVerts.idxOffset], buffData, accessor->count * sizeof(uint32_t));
					}
					else if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
						const uint16_t *buffData = reinterpret_cast<const uint16_t *>(buffer->data.data + bufferView->byte_offset + accessor->byte_offset);
						for (uint64_t idx = 0; idx < accessor->count; idx++) {
							meshVerts.indices[meshVerts.idxOffset + idx] = static_cast<uint32_t>(buffData[idx]);
						}
					}
					meshVerts.idxOffset += mesh.subMeshes[j].indexCount;
				}
			}

			meshes.push_back(std::move(mesh));
			meshIds[i] = meshes.size();
		}
		return meshIds;
	}
}
