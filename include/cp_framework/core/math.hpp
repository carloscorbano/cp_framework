#pragma once

#include "types.hpp"
#include "export.hpp"

// Vector and matrix helpers
namespace cp::math {

    /**
     * @brief Normalize a 2D vector.
     * @param v Input 2D vector.
     * @return Normalized 2D vector.
     */
    CP_API vec2 normalize(const vec2& v);

    /**
         * @brief Compute the magnitude (length) of a 2D vector.
         * @param v Input 2D vector.
         * @return Length as f32.
         */
    CP_API f32 length(const vec2& v);

    /**
     * @brief Compute the dot product of two 2D vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Scalar dot product.
     */
    CP_API f32 dot(const vec2& a, const vec2& b);

    /**
     * @brief Compute the cross product of two 3D vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Resulting perpendicular vector.
     */
    CP_API vec3 cross(const vec3& a, const vec3& b);

    /**
     * @brief Compute the dot product of two 3D vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Scalar dot product.
     */
    CP_API f32 dot(const vec3& a, const vec3& b);

    /**
     * @brief Normalize a 3D vector.
     * @param v Input 3D vector.
     * @return Normalized 3D vector.
     */
    CP_API vec3 normalize(const vec3& v);

    /**
     * @brief Compute the magnitude (length) of a 3D vector.
     * @param v Input 3D vector.
     * @return Length as f32.
     */
    CP_API f32 length(const vec3& v);

    /**
     * @brief Translate a matrix by a vector.
     * @param m Input matrix.
     * @param v Translation vector.
     * @return Translated matrix.
     */
    CP_API mat4 translate(const mat4& m, const vec3& v);

    /**
     * @brief Rotate a matrix around an axis.
     * @param m Input matrix.
     * @param angle Rotation angle in radians.
     * @param axis Rotation axis.
     * @return Rotated matrix.
     */
    CP_API mat4 rotate(const mat4& m, f32 angle, const vec3& axis);

    /**
     * @brief Scale a matrix by a vector.
     * @param m Input matrix.
     * @param s Scale vector.
     * @return Scaled matrix.
     */
    CP_API mat4 scale(const mat4& m, const vec3& s);

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
    CP_API mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);

    /**
     * @brief Create a perspective projection matrix.
     * @param fov Field of view in radians.
     * @param aspect Width/height aspect ratio.
     * @param near Near clipping plane.
     * @param far Far clipping plane.
     * @return Perspective projection matrix.
     */
    CP_API mat4 perspective(f32 fov, f32 aspect, f32 near, f32 far);

    /**
     * @brief Create a view matrix pointing from eye to center.
     * @param eye Camera origin.
     * @param center Target position.
     * @param up Up direction vector.
     * @return View transformation matrix.
     */
    CP_API mat4 lookAt(const vec3& eye, const vec3& center, const vec3& up);

    /**
     * @brief Convert degrees to radians.
     * @param degrees Angle in degrees.
     * @return Angle in radians.
     */
    CP_API f32 to_radians(f32 degrees);

    /**
     * @brief Convert radians to degrees.
     * @param radians Angle in radians.
     * @return Angle in degrees.
     */
    CP_API f32 to_degrees(f32 radians);

    /**
     * @brief Compute the inverse of a 4×4 matrix.
     * @param m Input matrix.
     * @return Inverted matrix.
     */
    CP_API mat4 inverse(const mat4& m);

    /**
     * @brief Transpose a 4×4 matrix.
     * @param m Input matrix.
     * @return Transposed matrix.
     */
    CP_API mat4 transpose(const mat4& m);

    /**
     * @brief Reflect a vector around a normal.
     * @param i Incident vector.
     * @param n Normal vector.
     * @return Reflected vector.
     */
    CP_API vec3 reflect(const vec3& i, const vec3& n);

    /**
     * @brief Compute the inverse of a 3×3 matrix.
     * @param m Input matrix.
     * @return Inverted 3×3 matrix.
     */
    CP_API mat3 inverse(const mat3& m);

    /**
     * @brief Transpose a 3×3 matrix.
     * @param m Input matrix.
     * @return Transposed 3×3 matrix.
     */
    CP_API mat3 transpose(const mat3& m);

    /**
     * @brief Return a 4×4 identity matrix.
     * @return Identity matrix.
     */
    CP_API mat4 identity();

    /**
     * @brief Return a 3×3 identity matrix.
     * @return Identity matrix.
     */
    CP_API mat3 identity3();

    /**
     * @brief Compute the distance between two 3D points.
     * @param a First point.
     * @param b Second point.
     * @return Distance as f32.
     */
    CP_API f32 distance(const vec3& a, const vec3& b);

    /**
     * @brief Compute the distance between two 2D points.
     * @param a First point.
     * @param b Second point.
     * @return Distance as f32.
     */
    CP_API f32 distance(const vec2& a, const vec2& b);

    /**
     * @brief Normalize a 4D vector.
     * @param v Input 4D vector.
     * @return Normalized 4D vector.
     */
    CP_API vec4 normalize(const vec4& v);

    /**
     * @brief Compute the magnitude (length) of a 4D vector.
     * @param v Input 4D vector.
     * @return Length as f32.
     */
    CP_API f32 length(const vec4& v);

    /**
     * @brief Compute the dot product of two 4D vectors.
     * @param a First vector.
     * @param b Second vector.
     * @return Scalar dot product.
     */
    CP_API f32 dot(const vec4& a, const vec4& b);

} // namespace cp::math