#ifndef MINI_GLM_GLM_HPP
#define MINI_GLM_GLM_HPP

namespace glm {

struct vec2 {
	float x{ 0.0f };
	float y{ 0.0f };

	constexpr vec2() = default;
	constexpr vec2(float xValue, float yValue) : x(xValue), y(yValue) {}

	constexpr vec2& operator+=(const vec2& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
};

constexpr vec2 operator+(vec2 left, const vec2& right) {
	left += right;
	return left;
}

constexpr vec2 operator-(const vec2& left, const vec2& right) {
	return { left.x - right.x, left.y - right.y };
}

constexpr vec2 operator*(const vec2& value, float scalar) {
	return { value.x * scalar, value.y * scalar };
}

constexpr vec2 operator*(float scalar, const vec2& value) {
	return value * scalar;
}

constexpr vec2 operator/(const vec2& value, float scalar) {
	return { value.x / scalar, value.y / scalar };
}

}

#endif
