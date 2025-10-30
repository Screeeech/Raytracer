#pragma once
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>

#include "ColorRGB.h"
#include "DataTypes.h"
#include "MathHelpers.h"
#include "Vector3.h"

namespace dae
{
namespace GeometryUtils
{
#pragma region Sphere HitTest

// SPHERE HIT-TESTS
inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
{
    // Quadratic equation
    // const float a{ Vector3::Dot(ray.direction, ray.direction) };
    // const float b{ 2 * Vector3::Dot(ray.direction, ray.origin - sphere.origin) };
    // const float c{ Vector3::Dot((ray.origin - sphere.origin), (ray.origin - sphere.origin)) -
    //                (sphere.radius * sphere.radius) };
    //
    // const float discriminant{ (b * b) - (4 * a * c) };
    // if(discriminant <= 0)
    //     return false;
    //
    // float t{ (-b - sqrtf(discriminant)) / (2 * a) };
    // if(t < ray.min)
    //     t = (-b + sqrtf(discriminant)) / (2 * a);

    // Geometric equation
    const Vector3 rayToSpehere{ sphere.origin - ray.origin };
    const float originRayDistanceSqr{ Vector3::Reject(rayToSpehere, ray.direction).SqrMagnitude() };

    if(originRayDistanceSqr >= (sphere.radius * sphere.radius))
        return false;

    const float tRayCenter{ Vector3::Dot(rayToSpehere, ray.direction) };
    const float tCenterHit{ sqrtf((sphere.radius * sphere.radius) - originRayDistanceSqr) };

    const float t1{ tRayCenter - tCenterHit };
    const float t2{ tRayCenter + tCenterHit };

    if(t1 < ray.min or t1 > ray.max)
        return false;

    if(ignoreHitRecord)
        return true;

    const Vector3 hitPoint{ ray.origin + (t1 * ray.direction) };
    hitRecord.origin = hitPoint;
    hitRecord.didHit = true;
    hitRecord.t = t1;
    hitRecord.materialIndex = sphere.materialIndex;
    hitRecord.normal = (hitPoint - sphere.origin).Normalized();

    return true;
}

inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
{
    HitRecord temp{};
    return HitTest_Sphere(sphere, ray, temp, true);
}

#pragma endregion
#pragma region Plane HitTest

// PLANE HIT-TESTS
inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
{
    const float t{ (Vector3::Dot((plane.origin - ray.origin), plane.normal)) / Vector3::Dot(ray.direction, plane.normal) };

    if(t < ray.min or t >= ray.max)
        return false;

    if(ignoreHitRecord)
        return true;

    const Vector3 hitPoint{ ray.origin + (t * ray.direction) };
    hitRecord.origin = hitPoint;
    hitRecord.didHit = true;
    hitRecord.t = t;
    hitRecord.materialIndex = plane.materialIndex;
    hitRecord.normal = plane.normal;

    return true;
}

inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
{
    HitRecord temp{};
    return HitTest_Plane(plane, ray, temp, true);
}

#pragma endregion
#pragma region Triangle HitTest

// TRIANGLE HIT-TESTS
inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
{
    const auto vn{ Vector3::Dot(ray.direction, triangle.normal) };
    if(AreEqual(vn, 0.f))
        return false;

    switch(triangle.cullMode)
    {
        case TriangleCullMode::FrontFaceCulling:
            if(vn > 0)
                return false;
            break;
        case TriangleCullMode::BackFaceCulling:
            if(vn < 0)
                return false;
            break;
        case TriangleCullMode::NoCulling:
            break;
    }

    const auto rayToVert{ triangle.v0 - ray.origin };
    const auto t{ Vector3::Dot(rayToVert, triangle.normal) / vn };

    if(t < ray.min or t > ray.max)
        return false;

    const auto hitPoint{ ray.origin + (ray.direction * t) };
    auto isPointOutTriangle = [hitPoint, triangle](const Vector3& vertex1, const Vector3& vertex2) -> bool
    {
        const Vector3 e{ vertex1, vertex2 };
        const Vector3 p{ vertex1, hitPoint };
        return Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0;
    };

    if(not(isPointOutTriangle(triangle.v0, triangle.v1) or isPointOutTriangle(triangle.v1, triangle.v2) or
           isPointOutTriangle(triangle.v2, triangle.v0)))
    {
        if(not ignoreHitRecord)
        {
            hitRecord.origin = hitPoint;
            hitRecord.didHit = true;
            hitRecord.t = t;
            hitRecord.materialIndex = triangle.materialIndex;
            hitRecord.normal = triangle.normal;
        }
        return true;
    }

    return false;
}

inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
{
    HitRecord temp{};
    return HitTest_Triangle(triangle, ray, temp, true);
}

#pragma endregion
#pragma region TriangeMesh HitTest

inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
{
    float tx1 = (mesh.minWorldAABB.x - ray.origin.x) / ray.direction.x;
    float tx2 = (mesh.maxWorldAABB.x - ray.origin.x) / ray.direction.x;

    float tmin = std::min(tx1, tx2);
    float tmax = std::max(tx1, tx2);

    float ty1 = (mesh.minWorldAABB.y - ray.origin.y) / ray.direction.y;
    float ty2 = (mesh.maxWorldAABB.y - ray.origin.y) / ray.direction.y;

    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    float tz1 = (mesh.minWorldAABB.z - ray.origin.z) / ray.direction.z;
    float tz2 = (mesh.maxWorldAABB.z - ray.origin.z) / ray.direction.z;

    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    return tmax > 0 and tmax >= tmin;
}

inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
{
    if(not SlabTest_TriangleMesh(mesh, ray))
        return false;

    size_t triIndex{};
    HitRecord closestHit;
    for(size_t i{}; i < mesh.indices.size(); i += 3)
    {
        Triangle tri{ mesh.transformedVertices[mesh.indices[i + 0]], mesh.transformedVertices[mesh.indices[i + 1]],
                      mesh.transformedVertices[mesh.indices[i + 2]], mesh.transformedNormals[triIndex] };
        ++triIndex;

        HitRecord currentHit{};
        HitTest_Triangle(tri, ray, currentHit);
        if(currentHit.t < closestHit.t)
            closestHit = currentHit;
    }

    hitRecord = closestHit;
    hitRecord.materialIndex = mesh.materialIndex;
    return false;
}

inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
{
    HitRecord temp{};
    return HitTest_TriangleMesh(mesh, ray, temp, true);
}

#pragma endregion
}  // namespace GeometryUtils

namespace LightUtils
{
// Direction from target to light
inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
{
    if(light.type == LightType::Point)
    {
        return light.origin - origin;
    }
    return -light.direction * FLT_MAX;
}

inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
{
    const Vector3 lightToTarget{ light.origin - target };
    switch(light.type)
    {
        case LightType::Point:
        {
            const ColorRGB Ergb{ light.color * (light.intensity / lightToTarget.SqrMagnitude()) };
            return Ergb;
        }
        case LightType::Directional:
        {
            const ColorRGB Ergb{ light.color * light.intensity };
            return Ergb;
        }
        default:
            return {};
    }
}
}  // namespace LightUtils

namespace Utils
{
// Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505)  // Warning unreferenced local function

static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<int>& indices,
                     std::vector<Vector3>& normals)
{
    std::ifstream file(filename);
    if(not file)
    {
        std::cerr << "The file: " << filename << " could not be loaded\n";
        return false;
    }

    std::string sCommand;
    // start a while iteration ending when the end of file is reached (ios::eof)
    while(!file.eof())
    {
        // read the first word of the string, use the >> operator
        // (istream::operator>>)
        sCommand = "";
        file >> sCommand;
        // use conditional statements to process the different commands
        if(sCommand == "#")
        {
            // Ignore Comment
        }
        else if(sCommand == "v")
        {
            // Vertex
            float x{};
            float y{};
            float z{};
            file >> x >> y >> z;
            positions.emplace_back(x, y, z);
        }
        else if(sCommand == "f")
        {
            float i0{};
            float i1{};
            float i2{};
            file >> i0 >> i1 >> i2;

            indices.push_back((int)i0 - 1);
            indices.push_back((int)i1 - 1);
            indices.push_back((int)i2 - 1);
        }
        // read till end of line and ignore all remaining chars
        file.ignore(1000, '\n');

        if(file.eof())
            break;
    }

    // Precompute normals
    for(uint64_t index = 0; index < indices.size(); index += 3)
    {
        uint32_t i0 = indices[index];
        uint32_t i1 = indices[index + 1];
        uint32_t i2 = indices[index + 2];

        Vector3 edgeV0V1 = positions[i1] - positions[i0];
        Vector3 edgeV0V2 = positions[i2] - positions[i0];
        Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

        if(std::isnan(normal.x))
        {
            int k = 0;
        }

        normal.Normalize();
        if(std::isnan(normal.x))
        {
            int k = 0;
        }

        normals.push_back(normal);
    }

    return true;
}

#pragma warning(pop)
}  // namespace Utils
}  // namespace dae
