#pragma once

#include <glm/glm.hpp>
#include "Camera.h"

namespace engine {

class FPSCamera : public Camera
{
public:
    FPSCamera(float fovDeg, float aspect, float nearPlane, float farPlane);

    // --- Camera interface ---
    const glm::mat4& view() const override { return m_view; }
    const glm::mat4& projection() const override { return m_proj; }
    void setAspectRatio(float aspect) override;

    // --- Position / orientation ---
    void setPosition(const glm::vec3& p) { m_position = p; updateVectors(); }
    glm::vec3 position() const override { return m_position; }

    // --- Orientation getters ---
    glm::vec3 forward() const override { return glm::normalize(m_front); }
    glm::vec3 right()   const { return glm::normalize(m_right); }
    glm::vec3 up()      const { return glm::normalize(m_up); }
    // --- Yaw/pitch getters ---
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }


    void setViewMatrix(const glm::mat4& view) { m_view = view; }

    // --- Movement API ---
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);
    void rotate(float dx, float dy);

    void updateVectors();

private:
    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;

    glm::vec3 m_position{0.0f, 0.0f, 3.0f};
    float m_yaw   = -90.0f;
    float m_pitch = 0.0f;

    glm::vec3 m_front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    glm::mat4 m_view{1.0f};
    glm::mat4 m_proj{1.0f};

    float m_speed = 5.0f;
    float m_sensitivity = 0.1f;
};

} // namespace engine
