#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Matrix.h"
#include "SDL_scancode.h"
#include "Timer.h"
#include "Vector3.h"

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
    float fovAngle{};

    Vector3 forward{ Vector3::UnitZ };
    Vector3 up{ Vector3::UnitY };
    Vector3 right{ Vector3::UnitX };

    float totalPitch{ 0.f };
    float totalYaw{ 0.f };

    Matrix cameraToWorld;

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
        static constexpr float rotSpeed{ PI };
        const float deltaTime = pTimer->GetElapsed();

        // Keyboard Input
        const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

        // Mouse Input
        int mouseX{};
        int mouseY{};
        const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

        Move(pKeyboardState, deltaTime);
        Rotate(mouseState, mouseX, mouseY, deltaTime);
    }

    void Move(const uint8_t* pKeyboardState, float deltaTime)
    {
        static constexpr float moveSpeed{ 3.f };

        if(pKeyboardState[SDL_SCANCODE_W])
            origin += forward * moveSpeed * deltaTime;
        if(pKeyboardState[SDL_SCANCODE_A])
            origin -= right * moveSpeed * deltaTime;
        if(pKeyboardState[SDL_SCANCODE_S])
            origin -= forward * moveSpeed * deltaTime;
        if(pKeyboardState[SDL_SCANCODE_D])
            origin += right * moveSpeed * deltaTime;
        if(pKeyboardState[SDL_SCANCODE_SPACE])
            origin += up * moveSpeed * deltaTime;
        if(pKeyboardState[SDL_SCANCODE_LSHIFT])
            origin -= up * moveSpeed * deltaTime;
    }

    void Rotate(const uint32_t& mouseState, int mouseX, int mouseY, float deltaTime)
    {
        static constexpr float rotSpeed{ PI / 16 };

        if(not(SDL_BUTTON_LEFT & mouseState))
            return;

        float deltaYaw{ static_cast<float>(mouseX) * rotSpeed * deltaTime };
        float deltaPitch{ static_cast<float>(-mouseY) * rotSpeed * deltaTime };

        totalYaw += deltaYaw;
        totalPitch += deltaPitch;

        forward = Matrix::CreateRotationY(totalYaw).TransformVector(
            Matrix::CreateRotationX(totalPitch).TransformVector(Vector3::UnitZ));
    }

    [[nodiscard]] float GetFov() const
    {
        return tanf((PI / 180.f) * fovAngle / 2);
    }
};
}  // namespace dae
