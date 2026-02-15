#pragma once

#include <engine/scene/FPSCamera.h>

namespace app
{
    class ICameraController
    {
    public:
        virtual ~ICameraController() = default;

        virtual void update(
            engine::FPSCamera& camera,
            float deltaTime,
            float mouseDeltaX,
            float mouseDeltaY
        ) = 0;
    };
}
