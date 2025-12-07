#pragma once

#include "cp_framework/core/types.hpp"
#include "cp_framework/core/export.hpp"

// Vector and matrix helpers
namespace cp::math
{

    constexpr float PI = glm::pi<float>();
    constexpr float TWO_PI = glm::two_pi<float>();
    constexpr float HALF_PI = glm::half_pi<float>();
    constexpr float DEG2RAD = PI / 180.0f;
    constexpr float RAD2DEG = 180.0f / PI;

    /**
     * @brief Normalize a 2D vector.
     * @param v Input 2D vector.
     * @return Normalized 2D vector.
     */
    vec2 normalize(const vec2 &v);

    /**
     * @brief Safely normalizes a 2D vector.
     *
     * Returns the normalized vector if its length is above epsilon,
     * otherwise returns the original vector without causing NaN or INF.
     *
     * @param v Input vector.
     * @param eps Minimum safe length threshold.
     * @return vec2 Normalized vector or original vector if too small.
     */
    inline vec2 safe_normalize(const vec2 &v, float eps = 1e-6f)
    {
        float len = glm::length(v);
        if (len > eps)
            return v / len;
        return v; // return unchanged (zero or near-zero)
    }

    /**
     * @brief Safely normalizes a 3D vector.
     *
     * Works the same way as safe_normalize(vec2).
     */
    inline vec3 safe_normalize(const vec3 &v, float eps = 1e-6f)
    {
        float len = glm::length(v);
        if (len > eps)
            return v / len;
        return v;
    }

    /**
     * @brief Compute the magnitude (length) of a 2D vector.
     * @param v Input 2D vector.
     * @return Length as f32.
     */
    f32 length(const vec2 &v);

    /**
     * @brief Compute the dot product of two 2D vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Scalar dot product.
     */
    f32 dot(const vec2 &a, const vec2 &b);

    /**
     * @brief Compute the cross product of two 3D vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Resulting perpendicular vector.
     */
    vec3 cross(const vec3 &a, const vec3 &b);

    /**
     * @brief Compute the dot product of two 3D vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Scalar dot product.
     */
    f32 dot(const vec3 &a, const vec3 &b);

    /**
     * @brief Normalize a 3D vector.
     * @param v Input 3D vector.
     * @return Normalized 3D vector.
     */
    vec3 normalize(const vec3 &v);

    /**
     * @brief Compute the magnitude (length) of a 3D vector.
     * @param v Input 3D vector.
     * @return Length as f32.
     */
    f32 length(const vec3 &v);

    /**
     * @brief Translate a matrix by a vector.
     * @param m Input matrix.
     * @param v Translation vector.
     * @return Translated matrix.
     */
    mat4 translate(const mat4 &m, const vec3 &v);

    /**
     * @brief Rotate a matrix around an axis.
     * @param m Input matrix.
     * @param angle Rotation angle in radians.
     * @param axis Rotation axis.
     * @return Rotated matrix.
     */
    mat4 rotate(const mat4 &m, f32 angle, const vec3 &axis);

    /**
     * @brief Scale a matrix by a vector.
     * @param m Input matrix.
     * @param s Scale vector.
     * @return Scaled matrix.
     */
    mat4 scale(const mat4 &m, const vec3 &s);

    /**
     * @brief Create an orthographic projection matrix.
     * @param left Left clipping plane.
     * @param right Right clipping plane.
     * @param bottom Bottom clipping plane.
     * @param top Top clipping plane.
     * @param near Near clipping plane.
     * @param far Far clipping plane.
     * @return Orthographic projection matrix.
     */
    mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);

    /**
     * @brief Create a perspective projection matrix.
     * @param fov Field of view in radians.
     * @param aspect Width/height aspect ratio.
     * @param near Near clipping plane.
     * @param far Far clipping plane.
     * @return Perspective projection matrix.
     */
    mat4 perspective(f32 fov, f32 aspect, f32 near, f32 far);

    /**
     * @brief Create a view matrix pointing from eye to center.
     * @param eye Camera origin.
     * @param center Target position.
     * @param up Up direction vector.
     * @return View transformation matrix.
     */
    mat4 lookAt(const vec3 &eye, const vec3 &center, const vec3 &up);

    /**
     * @brief Convert degrees to radians.
     * @param degrees Angle in degrees.
     * @return Angle in radians.
     */
    f32 to_radians(f32 degrees);

    /**
     * @brief Convert radians to degrees.
     * @param radians Angle in radians.
     * @return Angle in degrees.
     */
    f32 to_degrees(f32 radians);

    /**
     * @brief Compute the inverse of a 4×4 matrix.
     * @param m Input matrix.
     * @return Inverted matrix.
     */
    mat4 inverse(const mat4 &m);

    /**
     * @brief Transpose a 4×4 matrix.
     * @param m Input matrix.
     * @return Transposed matrix.
     */
    mat4 transpose(const mat4 &m);

    /**
     * @brief Reflect a vector around a normal.
     * @param i Incident vector.
     * @param n Normal vector.
     * @return Reflected vector.
     */
    vec3 reflect(const vec3 &i, const vec3 &n);

    /**
     * @brief Compute the inverse of a 3×3 matrix.
     * @param m Input matrix.
     * @return Inverted 3×3 matrix.
     */
    mat3 inverse(const mat3 &m);

    /**
     * @brief Transpose a 3×3 matrix.
     * @param m Input matrix.
     * @return Transposed 3×3 matrix.
     */
    mat3 transpose(const mat3 &m);

    /**
     * @brief Return a 4×4 identity matrix.
     * @return Identity matrix.
     */
    mat4 identity();

    /**
     * @brief Return a 3×3 identity matrix.
     * @return Identity matrix.
     */
    mat3 identity3();

    /**
     * @brief Compute the distance between two 3D points.
     * @param a First point.
     * @param b Second point.
     * @return Distance as f32.
     */
    f32 distance(const vec3 &a, const vec3 &b);

    /**
     * @brief Compute the distance between two 2D points.
     * @param a First point.
     * @param b Second point.
     * @return Distance as f32.
     */
    f32 distance(const vec2 &a, const vec2 &b);

    /**
     * @brief Normalize a 4D vector.
     * @param v Input 4D vector.
     * @return Normalized 4D vector.
     */
    vec4 normalize(const vec4 &v);

    /**
     * @brief Compute the magnitude (length) of a 4D vector.
     * @param v Input 4D vector.
     * @return Length as f32.
     */
    f32 length(const vec4 &v);

    /**
     * @brief Compute the dot product of two 4D vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Scalar dot product.
     */
    f32 dot(const vec4 &a, const vec4 &b);

} // namespace cp::math