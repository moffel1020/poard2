#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>

class Camera {
public:
    enum class Dir {
        Front,
        Back,
        Up,
        Down,
        Left,
        Right,
    };

    Camera(float aspect, float fov = 70.0f);

    Camera() = delete;
    Camera(const Camera& other) = default;
    Camera& operator=(const Camera& other) = default;
    Camera(Camera&& other) = default;
    Camera& operator=(Camera&& other) = default;

    void update();
    void rotate(float yawDiff, float pitchDiff);

    void setAspect(float width, float height) { proj = glm::perspective(fov, width / height, zNear, zFar); }
    const glm::mat4& getView() const { return view; }
    const glm::mat4& getProj() const { return proj; }
    float getYaw() { return yaw; };
    float getPitch() { return pitch; };

    template <Camera::Dir>
    void move(float);

private:
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 right;
    glm::vec3 up;

    float fov;
    float yaw;
    float pitch;

    glm::mat4 view;
    glm::mat4 proj;

    static constexpr float zNear = 0.1f;
    static constexpr float zFar = 10000.0f;
    static constexpr glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
};

template <>
inline void Camera::move<Camera::Dir::Front>(float distance) {
    position += distance * direction;
}
template <>
inline void Camera::move<Camera::Dir::Back>(float distance) {
    position -= distance * direction;
}
template <>
inline void Camera::move<Camera::Dir::Left>(float distance) {
    position -= distance * right;
}
template <>
inline void Camera::move<Camera::Dir::Right>(float distance) {
    position += distance * right;
}
template <>
inline void Camera::move<Camera::Dir::Up>(float distance) {
    position += distance * up;
}
template <>
inline void Camera::move<Camera::Dir::Down>(float distance) {
    position -= distance * up;
}
