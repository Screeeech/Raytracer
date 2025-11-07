#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.hpp"
#include "Matrix.hpp"
#include "SDL_scancode.h"
#include "Timer.hpp"
#include "Vector3.hpp"

namespace dae
{
struct Camera final
{
    Camera() = default;

    Camera(const Vector3& _origin, float _fovAngle)
        : origin{ _origin }
        , fov{ tanf((PI / 180.f) * _fovAngle / 2) }
    {
    }

    Vector3 origin;
    float fov{};

    Vector3 forward{ Vector3::UnitZ };
    Vector3 up{ Vector3::UnitY };
    Vector3 right{ Vector3::UnitX };

    float totalPitch{ 0.f };
    float totalYaw{ 0.f };

    Matrix cameraToWorld;

    void UpdateFOV(float fovAngle)
    {
        fov = tanf((PI / 180.f) * fovAngle / 2);
    }

    Matrix CalculateCameraToWorld()
    {
        right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
        up = Vector3::Cross(forward, right).Normalized();

        cameraToWorld = { { right.x, right.y, right.z, 0 },
                          { up.x, up.y, up.z, 0 },
                          { forward.x, forward.y, forward.z, 0 },
                          { origin.x, origin.y, origin.z, 1 } };

        return cameraToWorld;
    }

    void Update(Timer* pTimer)
    {
        static constexpr float rotSpeed{ 0.05 };
        static constexpr float moveSpeedMouse{ 0.75f };
        static constexpr float moveSpeedKeyboard{ 3.f };

        const float deltaTime = pTimer->GetElapsed();

        // Keyboard Input
        const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

        // Mouse Input
        int mouseX{};
        int mouseY{};
        const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

        // Panning with L+R
        if(mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
        {
            origin += right * deltaTime * moveSpeedMouse * static_cast<float>(mouseX);
            origin -= up * deltaTime * moveSpeedMouse * static_cast<float>(mouseY);
        }
        else if(mouseState == SDL_BUTTON_LMASK)
        {
            origin += forward * deltaTime * moveSpeedMouse * static_cast<float>(mouseY);
            Rotate(static_cast<float>(mouseX) * deltaTime * rotSpeed, 0);
        }
        else if(mouseState == SDL_BUTTON_RMASK)
        {
            Rotate(static_cast<float>(mouseX) * deltaTime * rotSpeed, -static_cast<float>(mouseY) * deltaTime * rotSpeed);
        }
        else
        {
            if(pKeyboardState[SDL_SCANCODE_W])
                origin += forward * moveSpeedKeyboard * deltaTime;
            if(pKeyboardState[SDL_SCANCODE_A])
                origin -= right * moveSpeedKeyboard * deltaTime;
            if(pKeyboardState[SDL_SCANCODE_S])
                origin -= forward * moveSpeedKeyboard * deltaTime;
            if(pKeyboardState[SDL_SCANCODE_D])
                origin += right * moveSpeedKeyboard * deltaTime;
        }
    }

    void Rotate(float deltaYaw, float deltaPitch)
    {
        totalYaw += deltaYaw;
        totalPitch += deltaPitch;

        forward = Matrix::CreateRotationY(totalYaw).TransformVector(
            Matrix::CreateRotationX(totalPitch).TransformVector(Vector3::UnitZ));
    }
};
}  // namespace dae
