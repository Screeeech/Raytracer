#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include <stdexcept>

#include "Math.h"
#include "Matrix.h"
#include "Timer.h"

namespace dae
{
struct Camera final
{
    Camera() = default;

    Camera(const Vector3& _origin, float _fovAngle)
        : origin{ _origin }
        , fovAngle{ _fovAngle }
    {
    }

    Vector3 origin;
    float fovAngle{ 90.f };

    Vector3 forward{ Vector3::UnitZ };
    Vector3 up{ Vector3::UnitY };
    Vector3 right{ Vector3::UnitX };

    float totalPitch{ 0.f };
    float totalYaw{ 0.f };

    Matrix cameraToWorld;

    Matrix CalculateCameraToWorld()
    {
        Matrix ONB{ { right.x, up.x, forward.x, origin.x },
                    { right.y, up.y, forward.y, origin.y },
                    { right.z, up.z, forward.z, origin.z },
                    { 0, 0, 0, 1 } };
        cameraToWorld = Matrix::Inverse(ONB);
        return cameraToWorld;
    }

    void Update(Timer* pTimer)
    {
        const float deltaTime = pTimer->GetElapsed();

        // Keyboard Input
        const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

        // Mouse Input
        int mouseX{};
        int mouseY{};
        const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
    }
};
}  // namespace dae
