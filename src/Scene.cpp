#include "Scene.hpp"

#include <algorithm>

#include "ColorRGB.hpp"
#include "DataTypes.hpp"
#include "Material.hpp"
#include "MathHelpers.hpp"
#include "Utils.hpp"

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

    for(const Triangle& triangle : m_Triangles)
    {
        GeometryUtils::HitTest_Triangle(triangle, ray, currentHit);
        if(currentHit.t < closestHit.t)
            closestHit = currentHit;
    }

    for(const TriangleMesh& mesh : m_TriangleMeshGeometries)
    {
        if(not GeometryUtils::SlabTest_TriangleMesh(mesh, ray))
            continue;

        GeometryUtils::HitTest_TriangleMesh(mesh, ray, currentHit);
        if(currentHit.t < closestHit.t)
            closestHit = currentHit;
    }
}

bool Scene::DoesHit(const Ray& ray) const
{
    return std::ranges::any_of(m_SphereGeometries.cbegin(), m_SphereGeometries.cend(),
                               [ray](const Sphere& sphere) { return GeometryUtils::HitTest_Sphere(sphere, ray); }) or
        std::ranges::any_of(m_PlaneGeometries.cbegin(), m_PlaneGeometries.cend(),
                            [ray](const Plane& plane) { return GeometryUtils::HitTest_Plane(plane, ray); }) or
        std::ranges::any_of(m_Triangles.cbegin(), m_Triangles.cend(),
                            [ray](const Triangle& triangle) { return GeometryUtils::HitTest_Triangle(triangle, ray); }) or
        std::ranges::any_of(m_TriangleMeshGeometries.cbegin(), m_TriangleMeshGeometries.cend(),
                            [ray](const TriangleMesh& mesh) { return GeometryUtils::HitTest_TriangleMesh(mesh, ray); });
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
    m_Camera.UpdateFOV(45.f);

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

void Scene_W3::Initialize()
{
    m_Camera.origin = { 0.f, 3.f, -9.f };
    m_Camera.UpdateFOV(45.f);

    auto const matCT_GrayRoughMetal{ AddMaterial(new Material_CookTorrence({ .r = .972f, .g = .960f, .b = .915f }, 1.f, 1.f)) };
    auto const matCT_GrayMediumMetal{ AddMaterial(new Material_CookTorrence({ .r = .972f, .g = .960f, .b = .915f }, 1.f, .6f)) };
    auto const matCT_GraySmoothMetal{ AddMaterial(new Material_CookTorrence({ .r = .972f, .g = .960f, .b = .915f }, 1.f, .1f)) };
    auto const matCT_GrayRoughPlastic{ AddMaterial(new Material_CookTorrence({ .r = .75f, .g = .75f, .b = .75f }, 0.f, 1.f)) };
    auto const matCT_GrayMediumPlastic{ AddMaterial(new Material_CookTorrence({ .r = .75f, .g = .75f, .b = .75f }, 0.f, .6f)) };
    auto const matCT_GraySmoothPlastic{ AddMaterial(new Material_CookTorrence({ .r = .75f, .g = .75f, .b = .75f }, 0.f, .1f)) };

    auto const matLambert_GrayBlue{ AddMaterial(new Material_Lambert({ .r = .49f, .g = .57f, .b = .57f }, 1.f)) };

    // Planes
    AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue);  // Back
    AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue);    // Bottom
    AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue);  // Top
    AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue);   // Right
    AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue);   // Left

    auto const matLambertPhong1{ AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.f)) };
    auto const matLambertPhong2{ AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.f)) };
    auto const matLambertPhong3{ AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 50.f)) };

    // Spheres
    AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
    AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
    AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
    AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
    AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
    AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

    // Light
    AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ .r = 1.f, .g = .61f, .b = .45f });    // Backlight
    AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ .r = 1.f, .g = .8f, .b = .45f });  // Front light left
    AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .r = .34f, .g = .47f, .b = .68f });
}

void Scene_W4_BunnyScene::Initialize()
{
    m_Camera.origin = { 0.f, 3.f, -9.f };
    m_Camera.UpdateFOV(45.f);

    unsigned char const matLambert_GrayBlue{ AddMaterial(new Material_Lambert({ .r = .49f, .g = .57f, .b = .57f }, 1.f)) };
    unsigned char const matLambert_White{ AddMaterial(new Material_Lambert(colors::White, 1.f)) };

    // Planes
    AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);  // Back
    AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);    // Bottom
    AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);  // Top
    AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);   // Right
    AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);   // Left


    const std::string filePath{ "resources/lowpoly_bunny.obj" };
    TriangleMesh* const pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
    Utils::ParseOBJ(filePath, pMesh->vertices, pMesh->indices, pMesh->normals);

    pMesh->Scale({ 2.f, 2.f, 2.f });
    pMesh->UpdateAABB();
    pMesh->UpdateTransforms();

    // Lights
    AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ .r = 1.f, .g = .61f, .b = .45f });    // Backlight
    AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ .r = 1.f, .g = .8f, .b = .45f });  // Front light left
    AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .r = .34f, .g = .47f, .b = .68f });
}

void Scene_W4_BunnyScene::Update(Timer* pTimer)
{
    Scene::Update(pTimer);

    float const rotation{ PI_DIV_2 * pTimer->GetTotal() };
    for(TriangleMesh& mesh : m_TriangleMeshGeometries)
    {
        mesh.RotateY(rotation);

        mesh.UpdateAABB();
        mesh.UpdateTransforms();
    }
}

void Scene_W4_ReferenceScene::Initialize()
{
    m_Camera.origin = { 0.f, 3.f, -9.f };
    m_Camera.UpdateFOV(45.f);

    unsigned char const matCT_GrayRoughMetal{ AddMaterial(
        new Material_CookTorrence({ .r = .972f, .g = .960f, .b = .915f }, 1.f, 1.f)) };
    unsigned char const matCT_GrayMediumMetal{ AddMaterial(
        new Material_CookTorrence({ .r = .972f, .g = .960f, .b = .915f }, 1.f, .6f)) };
    unsigned char const matCT_GraySmoothMetal{ AddMaterial(
        new Material_CookTorrence({ .r = .972f, .g = .960f, .b = .915f }, 1.f, .1f)) };
    unsigned char const matCT_GrayRoughPlastic{ AddMaterial(
        new Material_CookTorrence({ .r = .75f, .g = .75f, .b = .75f }, 0.f, 1.f)) };
    unsigned char const matCT_GrayMediumPlastic{ AddMaterial(
        new Material_CookTorrence({ .r = .75f, .g = .75f, .b = .75f }, 0.f, .6f)) };
    unsigned char const matCT_GraySmoothPlastic{ AddMaterial(
        new Material_CookTorrence({ .r = .75f, .g = .75f, .b = .75f }, 0.f, .1f)) };

    unsigned char const matLambert_GrayBlue{ AddMaterial(new Material_Lambert({ .r = .49f, .g = .57f, .b = .57f }, 1.f)) };
    unsigned char const matLambert_White{ AddMaterial(new Material_Lambert(colors::White, 1.f)) };

    // Planes
    AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue);  // Back
    AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matLambert_GrayBlue);    // Bottom
    AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue);  // Top
    AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue);   // Right
    AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matLambert_GrayBlue);   // Left

    // Spheres
    AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
    AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
    AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
    AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
    AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
    AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

    // Meshes
    Triangle const baseTriangle{ Vector3(-.75f, 1.5f, 0.f), Vector3(.75f, 0.f, 0.f), Vector3(-.75f, 0.f, 0.f) };
    m_TriangleMeshGeometries.reserve(3);
    AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
    m_TriangleMeshGeometries.back().AppendTriangle(baseTriangle, true);
    m_TriangleMeshGeometries.back().Translate({ -1.75f, 4.5f, 0.f });
    m_TriangleMeshGeometries.back().UpdateTransforms();

    AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
    m_TriangleMeshGeometries.back().AppendTriangle(baseTriangle, true);
    m_TriangleMeshGeometries.back().Translate({ 0.f, 4.5f, 0.f });
    m_TriangleMeshGeometries.back().UpdateTransforms();

    AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
    m_TriangleMeshGeometries.back().AppendTriangle(baseTriangle, true);
    m_TriangleMeshGeometries.back().Translate({ 1.75f, 4.5f, 0.f });
    m_TriangleMeshGeometries.back().UpdateTransforms();

    // Lights
    AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ .r = 1.f, .g = .61f, .b = .45f });    // Backlight
    AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ .r = 1.f, .g = .8f, .b = .45f });  // Front light left
    AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .r = .34f, .g = .47f, .b = .68f });
}

void Scene_W4_ReferenceScene::Update(Timer* pTimer)
{
    Scene::Update(pTimer);

    float const rotation{ PI_DIV_2 * pTimer->GetTotal() };
    for(TriangleMesh& mesh : m_TriangleMeshGeometries)
    {
        mesh.RotateY(rotation);

        mesh.UpdateAABB();
        mesh.UpdateTransforms();
    }
}

#pragma endregion
}  // namespace dae
