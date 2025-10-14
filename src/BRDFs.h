#pragma once
#include <cassert>
#include <cmath>
#include <stdexcept>

#include "ColorRGB.h"
#include "Math.h"
#include "MathHelpers.h"
#include "Vector3.h"

namespace dae::BRDF
{
/**
 * \param kd Diffuse Reflection Coefficient
 * \param cd Diffuse Color
 * \return Lambert Diffuse Color
 */
static ColorRGB Lambert(float kd, const ColorRGB& cd)
{
    assert(kd >= 0.f && kd <= 1.f);
    return (kd * cd) / PI;
}

static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
{
    return (kd * cd) / PI;
}

/**
 * \brief TODO:
 * \param ks Specular Reflection Coefficient
 * \param exp Phong Exponent
 * \param l Incoming (incident) Light Direction
 * \param v View Direction
 * \param n Normal of the Surface
 * \return Phong Specular Color
 */
static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
{
    const Vector3 reflect{ -l - (2 * Vector3::Dot(-l, n) * n) };
    const float specularReflectanceFactor{ ks * std::pow(Vector3::Dot(reflect, v), exp) };
    return ColorRGB{ .r = specularReflectanceFactor, .g = specularReflectanceFactor, .b = specularReflectanceFactor };
}

/**
 * \brief BRDF Fresnel Function >> Schlick
 * \param h Normalized Halfvector between View and Light directions
 * \param v Normalized View direction
 * \param f0 Base reflectivity of a surface based on IOR (Indices Of
 * Refrection), this is different for Dielectrics (Non-Metal) and Conductors
 * (Metal)
 * \return
 */
static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
{
    // TODO:: W3
    throw std::runtime_error("Not Implemented Yet");
    return {};
}

/**
 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation -
 * squared(roughness))
 * \param n Surface normal
 * \param h Normalized half vector
 * \param roughness Roughness of the material
 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
 */
static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
{
    // TODO:: W3
    throw std::runtime_error("Not Implemented Yet");
    return {};
}

/**
 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4
 * implementation - squared(roughness))
 * \param n Normal of the surface
 * \param v Normalized view direction
 * \param roughness Roughness of the material
 * \return BRDF Geometry Term using SchlickGGX
 */
static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
{
    // TODO:: W3
    throw std::runtime_error("Not Implemented Yet");
    return {};
}

/**
 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
 * \param n Normal of the surface
 * \param v Normalized view direction
 * \param l Normalized light direction
 * \param roughness Roughness of the material
 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) *
 * SchlickGGX(n,l,roughness))
 */
static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
{
    // TODO:: W3
    throw std::runtime_error("Not Implemented Yet");
    return {};
}

}  // namespace dae::BRDF
