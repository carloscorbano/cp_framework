#include "cp_framework/core/math.hpp"

namespace cp::math
{

	vec2 normalize(const vec2 &v) { return glm::normalize(v); }
	f32 length(const vec2 &v) { return glm::length(v); }
	f32 dot(const vec2 &a, const vec2 &b) { return glm::dot(a, b); }

	vec3 cross(const vec3 &a, const vec3 &b) { return glm::cross(a, b); }
	f32 dot(const vec3 &a, const vec3 &b) { return glm::dot(a, b); }
	vec3 normalize(const vec3 &v) { return glm::normalize(v); }
	f32 length(const vec3 &v) { return glm::length(v); }

	mat4 translate(const mat4 &m, const vec3 &v) { return glm::translate(m, v); }
	mat4 rotate(const mat4 &m, f32 angle, const vec3 &axis) { return glm::rotate(m, angle, axis); }
	mat4 scale(const mat4 &m, const vec3 &s) { return glm::scale(m, s); }

	mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
	{
		return glm::ortho(left, right, bottom, top, near, far);
	}

	mat4 perspective(f32 fov, f32 aspect, f32 near, f32 far) { return glm::perspective(fov, aspect, near, far); }

	mat4 lookAt(const vec3 &eye, const vec3 &center, const vec3 &up) { return glm::lookAt(eye, center, up); }

	f32 to_radians(f32 degrees) { return glm::radians(degrees); }
	f32 to_degrees(f32 radians) { return glm::degrees(radians); }

	mat4 inverse(const mat4 &m) { return glm::inverse(m); }
	mat4 transpose(const mat4 &m) { return glm::transpose(m); }

	vec3 reflect(const vec3 &i, const vec3 &n) { return glm::reflect(i, n); }

	mat3 inverse(const mat3 &m) { return glm::inverse(m); }
	mat3 transpose(const mat3 &m) { return glm::transpose(m); }

	mat4 identity() { return glm::identity<mat4>(); }
	mat3 identity3() { return glm::identity<mat3>(); }

	f32 distance(const vec3 &a, const vec3 &b) { return glm::distance(a, b); }
	f32 distance(const vec2 &a, const vec2 &b) { return glm::distance(a, b); }

	vec4 normalize(const vec4 &v) { return glm::normalize(v); }
	f32 length(const vec4 &v) { return glm::length(v); }
	f32 dot(const vec4 &a, const vec4 &b) { return glm::dot(a, b); }

} // namespace cp::math
