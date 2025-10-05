// External includes
#include "ColorRGB.h"
#include "DataTypes.h"
#include "MathHelpers.h"
#include "SDL.h"
#include "SDL_pixels.h"
#include "SDL_surface.h"

// Project includes
#include <iostream>

#include "Material.h"
#include "Math.h"
#include "Matrix.h"
#include "Renderer.h"
#include "Scene.h"
#include "Utils.h"
#include "Vector3.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow)
    : m_pWindow(pWindow)
    , m_pBuffer(SDL_GetWindowSurface(pWindow))
    , m_pBufferPixels(static_cast<uint32_t*>(m_pBuffer->pixels))
{
    // Initialize
    SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
}

void Renderer::Render(Scene* pScene) const
{
    Camera& camera = pScene->GetCamera();
    const auto& materials = pScene->GetMaterials();
    const auto& lights = pScene->GetLights();
    static const float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };

    for(int px{}; px < m_Width; ++px)
    {
        for(int py{}; py < m_Height; ++py)
        {
            // TODO: Optimise by caching FOV
            const float fov{ camera.GetFov() };

            // Get camera position
            const Vector3 ndc{ ((2.F * (static_cast<float>(px) + 0.5F) / static_cast<float>(m_Width)) - 1) *
                                   aspectRatio * fov,
                               (1 - ((2.F * (static_cast<float>(py) + 0.5F) / static_cast<float>(m_Width)) *
                                     aspectRatio)) *
                                   fov,
                               1 };

            const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

            const Vector3 localRayDirection{ (ndc).Normalized() };
            const Vector3 worldRayDirection = cameraToWorld.TransformVector(localRayDirection);

            const Ray viewRay{ .origin = camera.origin, .direction = worldRayDirection };

            HitRecord closestHit{};
            pScene->GetClosestHit(viewRay, closestHit);

            ColorRGB finalColor{};
            if(closestHit.didHit)
            {
                finalColor = materials[closestHit.materialIndex]->Shade();
            }
            finalColor.MaxToOne();

            m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(
                m_pBuffer->format, static_cast<uint8_t>(finalColor.r * 255),
                static_cast<uint8_t>(finalColor.g * 255), static_cast<uint8_t>(finalColor.b * 255));
        }
    }

    //@END
    // Update SDL Surface
    SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
    return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
