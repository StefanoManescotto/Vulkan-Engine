//
// Created by stefano on 04/08/26.
//

#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
// struct Transform {
// 	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
// 	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
// 	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
// };

// struct Camera;

class Transform {
public:
	glm::vec3 getPosition() const { return m_position; }
	glm::quat getRotation() const { return m_rotation; }
	glm::vec3 getScale() const { return m_scale; }

	void translate(const glm::vec3& delta);
    void rotateQuat(const glm::quat& delta);
	void rotateEuler(glm::vec3 eulerRotation);
	void scale(const glm::vec3& delta);

	glm::mat4 getTransformMatrix();
	[[nodiscard]] glm::mat4 getModelMatrix();
	// glm::mat4 TransformcalculateModelMatrix(Transform &transform, Camera &camera);

private:
	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::quat m_rotation = glm::quat(1, 0, 0, 0);
	glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::mat4 m_transformMat = glm::mat4(1);

	bool m_dirty = false;
};
