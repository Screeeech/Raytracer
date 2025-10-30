#pragma once
#include <algorithm>
#include <complex>
#include <cstdint>
#include <vector>

#include "ColorRGB.h"
#include "Matrix.h"
#include "Vector3.h"

namespace dae
{
#pragma region GEOMETRY

struct Sphere final
{
    Vector3 origin;
    float radius{};

    unsigned char materialIndex{ 0 };
};

struct Plane final
{
    Vector3 origin;
    Vector3 normal;

    unsigned char materialIndex{ 0 };
};

enum class TriangleCullMode : uint8_t
{
    FrontFaceCulling,
    BackFaceCulling,
    NoCulling
};

struct Triangle final
{
    Triangle() = default;

    Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal)
        : v0{ _v0 }
        , v1{ _v1 }
        , v2{ _v2 }
        , normal{ _normal.Normalized() }
    {
    }

    Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2)
        : v0{ _v0 }
        , v1{ _v1 }
        , v2{ _v2 }
    {
        const Vector3 edgeV0V1 = v1 - v0;
        const Vector3 edgeV0V2 = v2 - v0;
        normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
    }

    Vector3 v0;
    Vector3 v1;
    Vector3 v2;

    Vector3 normal;

    TriangleCullMode cullMode{};
    unsigned char materialIndex{};
};

struct TriangleMesh final
{
    TriangleMesh() = default;

    TriangleMesh(const std::vector<Vector3>& _vertices, const std::vector<int>& _indices, TriangleCullMode _cullMode)
        : vertices(_vertices)
        , indices(_indices)
        , cullMode(_cullMode)
    {
        // Calculate Normals
        CalculateNormals();

        // Update Transforms
        UpdateTransforms();
    }

    TriangleMesh(const std::vector<Vector3>& _vertices, const std::vector<int>& _indices, const std::vector<Vector3>& _normals,
                 TriangleCullMode _cullMode)
        : vertices(_vertices)
        , normals(_normals)
        , indices(_indices)
        , cullMode(_cullMode)
    {
        UpdateTransforms();
    }

    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<int> indices;
    unsigned char materialIndex{};

    TriangleCullMode cullMode{ TriangleCullMode::BackFaceCulling };

    Matrix rotationTransform;
    Matrix translationTransform;
    Matrix scaleTransform;

    std::vector<Vector3> transformedVertices;
    std::vector<Vector3> transformedNormals;


    Vector3 minObjectAABB;
    Vector3 maxObjectAABB;

    Vector3 minWorldAABB;
    Vector3 maxWorldAABB;

    void Translate(const Vector3& translation)
    {
        translationTransform = Matrix::CreateTranslation(translation);
    }

    void RotateY(float yaw)
    {
        rotationTransform = Matrix::CreateRotationY(yaw);
    }

    void Scale(const Vector3& scale)
    {
        scaleTransform = Matrix::CreateScale(scale);
    }

    void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
    {
        int startIndex = static_cast<int>(vertices.size());

        vertices.push_back(triangle.v0);
        vertices.push_back(triangle.v1);
        vertices.push_back(triangle.v2);

        indices.push_back(startIndex);
        indices.push_back(++startIndex);
        indices.push_back(++startIndex);

        normals.push_back(triangle.normal);

        // Not ideal, but making sure all vertices are updated
        if(!ignoreTransformUpdate)
            UpdateTransforms();
    }

    void CalculateNormals()
    {
        for(size_t i{}; i < indices.size(); i += 3)
        {
            const auto vert0{ vertices[indices[i + 0]] };
            const auto vert1{ vertices[indices[i + 1]] };
            const auto vert2{ vertices[indices[i + 2]] };

            const Vector3 vectorA{ vert0, vert1 };
            const Vector3 vectorB{ vert1, vert2 };
            normals.push_back((Vector3::Cross(vectorA, vectorB)).Normalized());
        }
    }

    void UpdateTransforms()
    {
        if(transformedVertices.size() < vertices.size() or transformedNormals.size() < normals.size())
        {
            transformedVertices.resize(vertices.size());
            transformedNormals.resize(normals.size());
        }

        Matrix finalTransform = scaleTransform * rotationTransform * translationTransform;

        transformedVertices.clear();
        transformedNormals.clear();

        for(auto const& vertex : vertices)
        {
            transformedVertices.emplace_back(finalTransform.TransformPoint(vertex));
        }

        for(auto const& normal : normals)
        {
            transformedNormals.emplace_back(rotationTransform.TransformPoint(normal));
        }

        UpdateTransformedAABB(finalTransform);
    }

    void UpdateAABB()
    {
        if(vertices.size() > 0)
        {
            minObjectAABB = vertices[0];
            maxObjectAABB = vertices[0];
            for(auto const& vertex : vertices)
            {
                minObjectAABB = Vector3::Min(vertex, minObjectAABB);
                maxObjectAABB = Vector3::Max(vertex, maxObjectAABB);
            }
        }
    }

    void UpdateTransformedAABB(const Matrix& finalTransform)
    {
        Vector3 tMinAABB = finalTransform.TransformPoint(minObjectAABB);
        Vector3 tMaxAABB = tMinAABB;

        Vector3 tAABB = finalTransform.TransformPoint(maxObjectAABB.x, minObjectAABB.y, minObjectAABB.z);
        tMinAABB = Vector3::Min(tAABB, tMinAABB);
        tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

        tAABB = finalTransform.TransformPoint(maxObjectAABB.x, minObjectAABB.y, maxObjectAABB.z);
        tMinAABB = Vector3::Min(tAABB, tMinAABB);
        tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

        tAABB = finalTransform.TransformPoint(minObjectAABB.x, minObjectAABB.y, maxObjectAABB.z);
        tMinAABB = Vector3::Min(tAABB, tMinAABB);
        tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

        tAABB = finalTransform.TransformPoint(minObjectAABB.x, maxObjectAABB.y, minObjectAABB.z);
        tMinAABB = Vector3::Min(tAABB, tMinAABB);
        tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

        tAABB = finalTransform.TransformPoint(maxObjectAABB.x, maxObjectAABB.y, minObjectAABB.z);
        tMinAABB = Vector3::Min(tAABB, tMinAABB);
        tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

        tAABB = finalTransform.TransformPoint(maxObjectAABB.x, maxObjectAABB.y, maxObjectAABB.z);
        tMinAABB = Vector3::Min(tAABB, tMinAABB);
        tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

        tAABB = finalTransform.TransformPoint(minObjectAABB.x, maxObjectAABB.y, maxObjectAABB.z);
        tMinAABB = Vector3::Min(tAABB, tMinAABB);
        tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

        minWorldAABB = tMinAABB;
        maxWorldAABB = tMaxAABB;
    }
};

#pragma endregion
#pragma region LIGHT
enum class LightType : uint8_t
{
    Point,
    Directional
};

struct Light final
{
    Vector3 origin;
    Vector3 direction;
    ColorRGB color{};
    float intensity{};

    LightType type{};
};

#pragma endregion
#pragma region MISC

struct Ray final
{
    Vector3 origin;
    Vector3 direction;

    float min{ 0.0001f };
    float max{ FLT_MAX };
};

struct HitRecord final
{
    Vector3 origin;
    Vector3 normal;
    float t = FLT_MAX;

    bool didHit{ false };
    unsigned char materialIndex{ 0 };
};

#pragma endregion
}  // namespace dae
