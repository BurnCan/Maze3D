#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera(float fovDeg, float aspect, float nearPlane, float farPlane);

    void setAspectRatio(float aspect);

    const glm::mat4& getView() const;
    const glm::mat4& getProjection() const;

    // temporary simple controls
    void setPosition(const glm::vec3& pos);
    void lookAt(const glm::vec3& target);

private:
    void updateView();
    void updateProjection();

private:
    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;

    glm::vec3 m_position{0.0f, 0.0f, 3.0f};
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};
};
