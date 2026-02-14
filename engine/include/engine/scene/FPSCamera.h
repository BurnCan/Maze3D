#pragma once

#include <glm/glm.hpp>
#include "Camera.h"

namespace engine
{

class FPSCamera : public Camera
{
public:
    FPSCamera(float fovDeg, float aspect, float nearPlane, float farPlane);



    // --- Camera interface ---
    const glm::mat4& view() const override;
    const glm::mat4& projection() const override;
    void setAspectRatio(float aspect) override;

    // --- FPS specific ---
    void setPosition(const glm::vec3& p);
    glm::vec3 position() const;

    // --- Movement API ---
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);
    void rotate(float dx, float dy);


private:
    void updateVectors();

private:
    // camera params
    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;

    // transform
    glm::vec3 m_position{0.0f, 0.0f, 3.0f};
    float m_yaw   = -90.0f;
    float m_pitch = 0.0f;

    // vectors
    glm::vec3 m_front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    // matrices
    glm::mat4 m_view{1.0f};
    glm::mat4 m_proj{1.0f};

    // tuning
    float m_speed = 5.0f;
    float m_sensitivity = 0.1f;
};

} // namespace engine
