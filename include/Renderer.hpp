#pragma once
#include <cstdint>
#include <vector>

#include "DataTypes.hpp"
#include "SDL_events.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
class Scene;

class Renderer final
{
public:
    explicit Renderer(SDL_Window* pWindow);
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) noexcept = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) noexcept = delete;

    void Render(Scene* pScene) const;
    [[nodiscard]] bool SaveBufferToImage() const;
    void ProcessInput(const SDL_Event& e);

    void CycleLightingMode();
    bool IsInShadow(const Scene* pScene, const Light& light, const HitRecord& closestHit) const;
    [[nodiscard]] ColorRGB CalculateLighting(const Scene* pScene, const HitRecord& closestHit) const;

    void ToggleShadows()
    {
        m_ShadowsEnabled = not m_ShadowsEnabled;
    }

private:
    enum class LightingMode : uint8_t
    {
        ObservedArea,  // Lambert cosine law
        Radiance,      // Incident radiance
        BRDF,          // Scattering of the light
        Combined,      // ObservedArea * Radiance * BRDF
        Count
    };

    LightingMode m_CurrentLightingMode{ LightingMode::Combined };
    bool m_ShadowsEnabled{ true };

    SDL_Window* m_pWindow{};

    SDL_Surface* m_pBuffer{};
    uint32_t* m_pBufferPixels{};

    int m_Width{};
    int m_Height{};
    std::vector<int> m_PixelIndices;
};
}  // namespace dae
