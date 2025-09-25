#include "camera.h"
#include "glm/ext/matrix_transform.hpp"
#include <algorithm>
#include <glm/glm.hpp>

Camera::Camera(float aspect, float fov)
    : position(glm::vec3(0.0f)),
      direction(glm::vec3(0.0f, 0.0f, -1.0f)),
      right(glm::vec3(1.0f, 0.0, 0.0f)),
      up(worldUp),
      fov(fov),
      yaw(-90.0f),
      pitch(0.0f),
      view(glm::lookAt(position, position + direction, up)),
      proj(glm::perspective(fov, aspect, zNear, zFar)) {}

void Camera::rotate(float yawDiff, float pitchDiff) {
    yaw += yawDiff;
    pitch += pitchDiff;
    pitch = std::clamp(pitch, -89.9f, 89.9f);
}

void Camera::update() {
    direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    direction.y = std::sin(glm::radians(pitch));
    direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    direction = glm::normalize(direction);

    right = glm::normalize(glm::cross(direction, worldUp));
    up = glm::normalize(glm::cross(right, direction));

    view = glm::lookAt(position, position + direction, up);
}
