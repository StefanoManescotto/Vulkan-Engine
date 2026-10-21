#include "transform.h"
#include <glm/common.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/trigonometric.hpp>

void Transform::translate(const glm::vec3& delta) {
    m_position += delta;
    m_dirty = true;
}

void Transform::rotateQuat(const glm::quat& delta) {
    m_rotation += delta;
    m_dirty = true;
}

void Transform::rotateEuler(glm::vec3 eulerRotation) {
    m_rotation = glm::quat(eulerRotation);
    m_rotation = glm::normalize(m_rotation);

    m_dirty = true;
}

[[nodiscard]] glm::mat4 Transform::getModelMatrix() {
    if(m_dirty) {
        glm::mat4 matTranslate = glm::translate(glm::mat4(1), m_position);
        glm::mat4 matRotate = glm::mat4_cast(m_rotation);
        glm::mat4 matScale = glm::scale(glm::mat4(1), m_scale);
        m_transformMat = matTranslate * matRotate * matScale;
        m_dirty = false;
    }

    return m_transformMat;
}


// Change name to calculateMVP (?)
// glm::mat4 Transform::calculateModelMatrix(Transform &transform, Camera &camera) {
//     // transform->rotate(glm::vec3(20,45,0));
//     glm::mat4 model = getModelMatrix();

//     glm::mat4 mvp = camera.projection * camera.view * model;
//     return mvp;
// }
