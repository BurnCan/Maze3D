#pragma once

#include <mesh_sculpt/render/FPSCamera.h>

namespace mesh_sculpt::app
{
    class ICameraController
    {
    public:
        virtual ~ICameraController() = default;

        virtual void update(
            mesh_sculpt::render::FPSCamera& camera,
            float deltaTime,
            float mouseDeltaX,
            float mouseDeltaY
        ) = 0;
    };
}
