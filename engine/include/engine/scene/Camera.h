#pragma once

#include <glm/glm.hpp>

namespace engine
{

class Camera
{
public:
    virtual ~Camera() = default;

    //  Interface
    virtual const glm::mat4& view() const = 0;
    virtual const glm::mat4& projection() const = 0;
    virtual void setAspectRatio(float aspect) = 0;

    // --- Virtual getters ---
    virtual glm::vec3 position() const = 0;
    virtual glm::vec3 forward() const = 0;


};

} // namespace engine
