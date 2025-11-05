
#include "Renderer.hpp"

#include <algorithm>
#include <cstdint>
#include <execution>
#include <numeric>

#include "ColorRGB.hpp"
#include "DataTypes.hpp"
#include "Material.hpp"
#include "Matrix.hpp"
#include "Scene.hpp"
#include "SDL_events.h"
#include "SDL_surface.h"
#include "Utils.hpp"
#include "Vector3.hpp"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow)
    : m_pWindow(pWindow)
    , m_pBuffer(SDL_GetWindowSurface(pWindow))
    , m_pBufferPixels(static_cast<uint32_t*>(m_pBuffer->pixels))
{
    // Initialize
    SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
    const uint32_t pixelCount{ static_cast<uint32_t>(m_Width * m_Height) };
    m_PixelIndices.resize(pixelCount);
    std::iota(m_PixelIndices.begin(), m_PixelIndices.end(), 0);
}

void Renderer::Render(Scene* pScene) const
{
    Camera& camera = pScene->GetCamera();
    static const float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };
    const float fov{ camera.fov };


    std::for_each(
        std::execution::par, m_PixelIndices.begin(), m_PixelIndices.end(),
        [&](const uint32_t pixelIdx)
        {
            const auto px{ pixelIdx % m_Width };
            const auto py{ pixelIdx / m_Width };

            // Get camera position
            const Vector3 ndc{ ((2.F * (static_cast<float>(px) + 0.5F) / static_cast<float>(m_Width)) - 1) * aspectRatio * fov,
                               (1 - ((2.F * (static_cast<float>(py) + 0.5F) / static_cast<float>(m_Width)) * aspectRatio)) * fov,
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
                finalColor = CalculateLighting(pScene, closestHit);
            }
            finalColor.MaxToOne();

            m_pBufferPixels[px + (py * m_Width)] =
                SDL_MapRGB(m_pBuffer->format, static_cast<uint8_t>(finalColor.r * 255), static_cast<uint8_t>(finalColor.g * 255),
                           static_cast<uint8_t>(finalColor.b * 255));
        });

    //@END
    // Update SDL Surface
    SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
    return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
    switch(m_CurrentLightingMode)
    {
        case LightingMode::ObservedArea:
            m_CurrentLightingMode = LightingMode::Radiance;
            break;
        case LightingMode::Radiance:
            m_CurrentLightingMode = LightingMode::BRDF;
            break;
        case LightingMode::BRDF:
            m_CurrentLightingMode = LightingMode::Combined;
            break;
        case LightingMode::Combined:
        case LightingMode::Count:
            m_CurrentLightingMode = LightingMode::ObservedArea;
            break;
    }
}

bool Renderer::IsInShadow(const Scene* pScene, const Light& light, const HitRecord& closestHit) const
{
    if(not m_ShadowsEnabled)
        return false;

    Vector3 hitToLight{ LightUtils::GetDirectionToLight(light, closestHit.origin + (closestHit.normal * 0.01f)) };
    const float hitToLightDistance{ hitToLight.Normalize() };
    const Ray hitToLightRay{ .origin = closestHit.origin, .direction = hitToLight, .max = hitToLightDistance };

    const float lightDot{ Vector3::Dot(closestHit.normal, hitToLight) };

    return lightDot < 0 or pScene->DoesHit(hitToLightRay);
}

ColorRGB Renderer::CalculateLighting(const Scene* pScene, const HitRecord& closestHit) const
{
    const auto& materials = pScene->GetMaterials();
    const auto& lights = pScene->GetLights();

    ColorRGB lighting{};
    for(const auto& light : lights)
    {
        if(IsInShadow(pScene, light, closestHit))
            continue;

        const Vector3 hitToCamera{ (pScene->GetCameraOrigin() - closestHit.origin).Normalized() };
        const Vector3 hitToLight{ (light.origin - closestHit.origin).Normalized() };
        const float observedArea{ Vector3::Dot(closestHit.normal, hitToLight) };

        const ColorRGB Ergb{ LightUtils::GetRadiance(light, closestHit.origin) };
        const ColorRGB BRDFrgb{ pScene->GetMaterials()[closestHit.materialIndex]->Shade(closestHit, hitToLight, hitToCamera) };


        switch(m_CurrentLightingMode)
        {
            case LightingMode::ObservedArea:
                lighting += Ergb * observedArea;
            case LightingMode::Radiance:
                lighting += Ergb;
            case LightingMode::BRDF:
                lighting += BRDFrgb;
            case LightingMode::Combined:
                lighting += Ergb * BRDFrgb * observedArea;
            default:
                break;
        }
    }
    return lighting;
}

void Renderer::ProcessInput(const SDL_Event& e)
{
    if(e.type == SDL_KEYUP)
    {
        switch(e.key.keysym.scancode)
        {
            case SDL_SCANCODE_F2:
                ToggleShadows();
                break;
            case SDL_SCANCODE_F3:
                CycleLightingMode();
                break;
            default:
                break;
        }
    }
}
