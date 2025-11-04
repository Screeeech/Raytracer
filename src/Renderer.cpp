
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
    const auto& materials = pScene->GetMaterials();
    const auto& lights = pScene->GetLights();
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
                for(const auto& light : lights)
                {
                    Vector3 hitToLight{ LightUtils::GetDirectionToLight(light, closestHit.origin + (closestHit.normal * 0.01f)) };
                    const float hitToLightDistance{ hitToLight.Normalize() };
                    const Ray hitToLightRay{ .origin = closestHit.origin, .direction = hitToLight, .max = hitToLightDistance };

                    const Vector3 hitToCamera{ (camera.origin - closestHit.origin).Normalized() };

                    const float lightDot{ Vector3::Dot(closestHit.normal, hitToLight) };
                    if(lightDot < 0 or pScene->DoesHit(hitToLightRay))
                        continue;

                    const ColorRGB Ergb{ LightUtils::GetRadiance(light, closestHit.origin) };
                    const ColorRGB BRDFrgb{ pScene->GetMaterials()[closestHit.materialIndex]->Shade(closestHit, hitToLight,
                                                                                                    hitToCamera) };
                    finalColor += BRDFrgb * Ergb * lightDot;
                }
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
    m_CurrentLightingMode = static_cast<Renderer::LightingMode>((static_cast<uint8_t>(m_CurrentLightingMode) + 1) %
                                                                static_cast<uint8_t>(Renderer::LightingMode::Count));
}
