//
// Created by stefano on 11/09/26.
//

#pragma once
#include <glm/fwd.hpp>

#include "scene/transform.h"
#include "scene/camera.h"

namespace TransformSystem {
    void translate(Transform& transform, const glm::vec3& delta);
    void rotate(Transform& transform, const glm::vec3& delta);

    [[nodiscard]] glm::mat4 getModelMatrix(Transform &transform);

    // TODO: Put it into a camera system (?)
    glm::mat4 calculateModelMatrix(Transform &transform, Camera &camera);
}
