#include "transform.h"

#include <glm/common.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/trigonometric.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

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

void Transform::scale(const glm::vec3& delta){
    m_scale += delta;
    m_dirty = true;
}

void Transform::setTransform(glm::mat4 transformMat) {
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(transformMat, m_scale, m_rotation, m_position, skew, perspective);
    m_transformMat = transformMat;
    m_dirty = false;
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
