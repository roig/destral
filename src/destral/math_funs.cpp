#include "math_funs.h"

#pragma warning( push )
#pragma warning(disable : 4201) // 
#include "glm/gtx/compatibility.hpp"
#pragma warning( pop )

namespace ds {

float map_range_clamped(const glm::vec2& from, const glm::vec2& to, float alpha) {
    return glm::lerp(to.x, to.y, glm::clamp(lerp_inverted(from.x, from.y, alpha), 0.0f, 1.0f));
}

glm::vec2 map_range_clamped(const glm::vec2& from, const glm::vec2& to, const glm::vec2& alpha) {
    glm::vec2 r;
    r.x = glm::lerp(to.x, to.y, glm::clamp(lerp_inverted(from.x, from.y, alpha.x), 0.0f, 1.0f));
    r.y = glm::lerp(to.x, to.y, glm::clamp(lerp_inverted(from.x, from.y, alpha.y), 0.0f, 1.0f));
    return r;
}


glm::vec2 map_range_clamped(const glm::vec2& from_min, const glm::vec2& from_max, const glm::vec2& to_min, const glm::vec2& to_max, const glm::vec2& alpha) {
    return glm::lerp(to_min, to_max, glm::clamp(lerp_inverted(from_min, from_max, alpha), 0.0f, 1.0f));
}

rect map_range_clamped(const rect& from, const rect& to, const rect alpha) {
    rect remapped;
    remapped.min = map_range_clamped(from.min, from.max, to.min, to.max, alpha.min);
    remapped.max = map_range_clamped(from.min, from.max, to.min, to.max, alpha.max);
    return remapped;
}

float map_range_unclamped(const glm::vec2& from, const glm::vec2& to, float alpha) {
    return glm::lerp(to.x, to.y, lerp_inverted(from.x, from.y, alpha));
}

glm::vec2 map_range_unclamped(const glm::vec2& from, const glm::vec2& to, const glm::vec2& alpha) {
    glm::vec2 r;
    r.x = glm::lerp(to.x, to.y, lerp_inverted(from.x, from.y, alpha.x));
    r.y = glm::lerp(to.x, to.y, lerp_inverted(from.x, from.y, alpha.y));
    return r;
}

glm::vec2 map_range_unclamped(const glm::vec2& from_min, const glm::vec2& from_max, const glm::vec2& to_min, const glm::vec2& to_max, const glm::vec2& alpha) {
    return glm::lerp(to_min, to_max, lerp_inverted(from_min, from_max, alpha));
}

rect map_range_unclamped(const rect& from, const rect& to, const rect alpha) {
    rect remapped;
    remapped.min = map_range_unclamped(from.min, from.max, to.min, to.max, alpha.min);
    remapped.max = map_range_unclamped(from.min, from.max, to.min, to.max, alpha.max);
    return remapped;
}

}