#pragma once
#include <vector>

#include "Camera.hpp"
#include "DataTypes.hpp"

namespace dae
{
// Forward Declarations
class Timer;
class Material;
struct Plane;
struct Sphere;
struct Light;

// Scene Base Class
class Scene
{
public:
    Scene();
    virtual ~Scene();

    Scene(const Scene&) = delete;
    Scene(Scene&&) noexcept = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) noexcept = delete;

    virtual void Initialize() = 0;

    virtual void Update(dae::Timer* pTimer)
    {
        m_Camera.Update(pTimer);
    }

    Camera& GetCamera()
    {
        return m_Camera;
    }

    void GetClosestHit(const Ray& ray, HitRecord& closestHit) const;
    [[nodiscard]] bool DoesHit(const Ray& ray) const;

    [[nodiscard]] const std::vector<Plane>& GetPlaneGeometries() const
    {
        return m_PlaneGeometries;
    }

    [[nodiscard]] const std::vector<Sphere>& GetSphereGeometries() const
    {
        return m_SphereGeometries;
    }

    [[nodiscard]] const std::vector<Light>& GetLights() const
    {
        return m_Lights;
    }

    [[nodiscard]] std::vector<Material*> GetMaterials() const
    {
        return m_Materials;
    }

protected:
    std::vector<Plane> m_PlaneGeometries;
    std::vector<Sphere> m_SphereGeometries;
    std::vector<TriangleMesh> m_TriangleMeshGeometries;
    std::vector<Triangle> m_Triangles;
    std::vector<Light> m_Lights;
    std::vector<Material*> m_Materials;

    Camera m_Camera;

    Sphere* AddSphere(const Vector3& origin, float radius, unsigned char materialIndex = 0);
    Plane* AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex = 0);
    TriangleMesh* AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex = 0);

    Light* AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color);
    Light* AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color);
    unsigned char AddMaterial(Material* pMaterial);
};

//+++++++++++++++++++++++++++++++++++++++++
// WEEK 1 Test Scene
class Scene_W1 final : public Scene
{
public:
    Scene_W1() = default;
    ~Scene_W1() override = default;

    Scene_W1(const Scene_W1&) = delete;
    Scene_W1(Scene_W1&&) noexcept = delete;
    Scene_W1& operator=(const Scene_W1&) = delete;
    Scene_W1& operator=(Scene_W1&&) noexcept = delete;

    void Initialize() override;
};

class Scene_W2 final : public Scene
{
public:
    Scene_W2() = default;
    ~Scene_W2() override = default;

    Scene_W2(Scene_W2&&) = delete;
    Scene_W2(const Scene_W2&) = delete;
    Scene_W2& operator=(Scene_W2&&) = delete;
    Scene_W2& operator=(const Scene_W2&) = delete;

    void Initialize() override;
};

class Scene_W3 final : public Scene
{
public:
    Scene_W3() = default;
    ~Scene_W3() override = default;

    Scene_W3(Scene_W3&&) = delete;
    Scene_W3(const Scene_W3&) = delete;
    Scene_W3& operator=(Scene_W3&&) = delete;
    Scene_W3& operator=(const Scene_W3&) = delete;

    void Initialize() override;
};

class Scene_W4_BunnyScene final : public Scene
{
public:
    Scene_W4_BunnyScene() = default;
    ~Scene_W4_BunnyScene() override = default;

    Scene_W4_BunnyScene(Scene_W4_BunnyScene&&) = delete;
    Scene_W4_BunnyScene(const Scene_W4_BunnyScene&) = delete;
    Scene_W4_BunnyScene& operator=(Scene_W4_BunnyScene&&) = delete;
    Scene_W4_BunnyScene& operator=(const Scene_W4_BunnyScene&) = delete;

    void Initialize() override;
    void Update(Timer* pTimer) override;
};

class Scene_W4_ReferenceScene final : public Scene
{
public:
    Scene_W4_ReferenceScene() = default;
    ~Scene_W4_ReferenceScene() override = default;

    Scene_W4_ReferenceScene(Scene_W4_ReferenceScene&&) = delete;
    Scene_W4_ReferenceScene(const Scene_W4_ReferenceScene&) = delete;
    Scene_W4_ReferenceScene& operator=(Scene_W4_ReferenceScene&&) = delete;
    Scene_W4_ReferenceScene& operator=(const Scene_W4_ReferenceScene&) = delete;

    void Initialize() override;
    void Update(Timer* pTimer) override;
};

}  // namespace dae
