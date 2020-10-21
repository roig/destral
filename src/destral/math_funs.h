#pragma once
#include "rect.h"

namespace ds {
	template<class T>
	inline T lerp_inverted(const T& a, const T& b, const T& lerp) {
		return (lerp - a) / (b - a);
	}

	float map_range_clamped(const glm::vec2& from, const glm::vec2& to, float alpha);
	glm::vec2 map_range_clamped(const glm::vec2& from, const glm::vec2& to, const glm::vec2& alpha);
	glm::vec2 map_range_clamped(const glm::vec2& from_min, const glm::vec2& from_max, const glm::vec2& to_min, const glm::vec2& to_max, const glm::vec2& alpha);
	rect map_range_clamped(const rect& from, const rect& to, const rect alpha);

	float map_range_unclamped(const glm::vec2& from, const glm::vec2& to, float alpha);
	glm::vec2 map_range_unclamped(const glm::vec2& from, const glm::vec2& to, const glm::vec2& alpha);
	glm::vec2 map_range_unclamped(const glm::vec2& from_min, const glm::vec2& from_max, const glm::vec2& to_min, const glm::vec2& to_max, const glm::vec2& alpha);
	rect map_range_unclamped(const rect& from, const rect& to, const rect alpha);

}