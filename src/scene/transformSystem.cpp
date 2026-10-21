//
// Created by stefano on 11/09/26.
//

#include "transformSystem.h"

// namespace TransformSystem {
//     void translate(Transform& transform, const glm::vec3& delta) {
//         transform.position += delta;
//     }

//     void rotate(Transform& transform, const glm::vec3& delta) {
//         transform.rotation += delta;
//     }

//     [[nodiscard]] glm::mat4 getModelMatrix(Transform &transform) {
//         auto model = glm::mat4(1.0f);
//         model = glm::translate(model, transform.position);
//         model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
//         model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
//         model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
//         model = glm::scale(model, transform.scale);
//         return model;
//     }

//     glm::mat4 calculateModelMatrix(Transform &transform, Camera &camera) {
//         // transform->rotate(glm::vec3(20,45,0));
//         glm::mat4 model = getModelMatrix(transform);

//         glm::mat4 mvp = camera.projection * camera.view * model;
//         return mvp;
//     }
// }
    