#pragma once

#include <veometri/render/FPSCamera.h>

namespace veometri::app
{
    class ICameraController
    {
    public:
        virtual ~ICameraController() = default;

        virtual void update(
            veometri::render::FPSCamera& camera,
            float deltaTime,
            float mouseDeltaX,
            float mouseDeltaY
        ) = 0;
    };
}
