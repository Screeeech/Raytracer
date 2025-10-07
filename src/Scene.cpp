#include "Scene.h"

#include <algorithm>

#include "DataTypes.h"
#include "Material.h"
#include "Utils.h"

namespace dae
{

#pragma region Base Scene

// Initialize Scene with Default Solid Color Material (RED)
Scene::Scene()
    : m_Materials({ new Material_SolidColor({ .r = 1, .g = 0, .b = 0 }) })
{
    m_SphereGeometries.reserve(32);
    m_PlaneGeometries.reserve(32);
    m_TriangleMeshGeometries.reserve(32);
    m_Lights.reserve(32);
}

Scene::~Scene()
{
    for(auto& pMaterial : m_Materials)
    {
        delete pMaterial;
        pMaterial = nullptr;
    }

    m_Materials.clear();
}

void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
{
    HitRecord currentHit{};
    for(const Sphere& sphere : m_SphereGeometries)
    {
        GeometryUtils::HitTest_Sphere(sphere, ray, currentHit);
        if(currentHit.t < closestHit.t)
            closestHit = currentHit;
    }
    for(const Plane& plane : m_PlaneGeometries)
    {
        GeometryUtils::HitTest_Plane(plane, ray, currentHit);
        if(currentHit.t < closestHit.t)
            closestHit = currentHit;
    }
}

bool Scene::DoesHit(const Ray& ray) const
{
    return std::ranges::any_of(m_SphereGeometries.cbegin(), m_SphereGeometries.cend(),
                               [ray](const Sphere& sphere) { return GeometryUtils::HitTest_Sphere(sphere, ray); }) or
        std::ranges::any_of(m_PlaneGeometries.cbegin(), m_PlaneGeometries.cend(),
                            [ray](const Plane& plane) { return GeometryUtils::HitTest_Plane(plane, ray); });
}

#pragma region Scene Helpers

Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
{
    Sphere s;
    s.origin = origin;
    s.radius = radius;
    s.materialIndex = materialIndex;

    m_SphereGeometries.emplace_back(s);
    return &m_SphereGeometries.back();
}

Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
{
    Plane p;
    p.origin = origin;
    p.normal = normal;
    p.materialIndex = materialIndex;

    m_PlaneGeometries.emplace_back(p);
    return &m_PlaneGeometries.back();
}

TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
{
    TriangleMesh m{};
    m.cullMode = cullMode;
    m.materialIndex = materialIndex;

    m_TriangleMeshGeometries.emplace_back(m);
    return &m_TriangleMeshGeometries.back();
}

Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
{
    Light l;
    l.origin = origin;
    l.intensity = intensity;
    l.color = color;
    l.type = LightType::Point;

    m_Lights.emplace_back(l);
    return &m_Lights.back();
}

Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
{
    Light l;
    l.direction = direction;
    l.intensity = intensity;
    l.color = color;
    l.type = LightType::Directional;

    m_Lights.emplace_back(l);
    return &m_Lights.back();
}

unsigned char Scene::AddMaterial(Material* pMaterial)
{
    m_Materials.push_back(pMaterial);
    return static_cast<unsigned char>(m_Materials.size() - 1);
}

#pragma endregion
#pragma endregion

#pragma region SCENE W1

void Scene_W1::Initialize()
{
    // default: Material id0 >> SolidColor Material (RED)
    constexpr unsigned char matId_Solid_Red = 0;
    const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

    const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
    const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
    const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

    // Spheres
    AddSphere({ -25.F, 0.F, 100.F }, 50.F, matId_Solid_Red);
    AddSphere({ 25.F, 0.F, 100.F }, 50.F, matId_Solid_Blue);

    // Plane
    AddPlane({ -75.F, 0.F, 0.F }, { 1.F, 0.F, 0.F }, matId_Solid_Green);
    AddPlane({ 75.F, 0.F, 0.F }, { -1.F, 0.F, 0.F }, matId_Solid_Green);
    AddPlane({ 0.F, -75.F, 0.F }, { 0.F, 1.F, 0.F }, matId_Solid_Yellow);
    AddPlane({ 0.F, 75.F, 0.F }, { 0.F, -1.F, 0.F }, matId_Solid_Yellow);
    AddPlane({ 0.F, 0.F, 125.F }, { 0.F, 0.F, -1.F }, matId_Solid_Magenta);
}

void Scene_W2::Initialize()
{
    m_Camera.origin = { 0.f, 3.f, -9.f };
    m_Camera.fovAngle = 45.f;
    // default: Material ide >> SolidColor Material (RED)
    constexpr unsigned char matId_Solid_Red = 0;
    const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
    const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
    const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
    const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });
    // Plane
    AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matId_Solid_Green);
    AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matId_Solid_Green);
    AddPlane({ 0.f, 0.f, 8.f }, { 0.f, 1.f, 0.f }, matId_Solid_Yellow);
    AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matId_Solid_Yellow);
    AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matId_Solid_Magenta);
    // Spheres
    AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
    AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
    AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
    AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
    AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
    AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
    // Light
    AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
}

#pragma endregion
}  // namespace dae
