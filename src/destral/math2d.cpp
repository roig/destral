#include "math2d.h"

#pragma warning( push )
#pragma warning(disable : 4201) // 
#include "glm/gtx/compatibility.hpp"
#pragma warning( pop )

namespace ds {

glm::vec2 rect_bottom_left(rect* r) {
    return { r->min.x, r->max.y };
}

glm::vec2 rect_top_right(rect* r) {
    return { r->max.x, r->min.y };
}

glm::vec2 rect_top_left(rect* r) {
    return r->min;
}

glm::vec2 rect_bottom_right(rect* r) {
    return r->max;
}

float rect_left(rect* r) {
    return r->min.x;
}
float rect_right(rect* r) {
    return r->max.x;
}
float rect_top(rect* r) {
    return r->min.y;
}
float rect_bottom(rect* r) {
    return r->max.y;
}

glm::vec2 rect_size(rect* r) { return r->max - r->min; }

glm::vec2 rect_center(rect* r) { return (r->min + r->max) / 2.0f; }

rect rect_from_size(const glm::vec2 min, const glm::vec2 size) {
    return { min, min + size };
}

rect rect_from_center(const glm::vec2 center, const glm::vec2 halfSize) {
    return { center - halfSize, center + halfSize };
}

rect rect_translated(rect* r, const glm::vec2 vector) {
    return { r->min + vector, r->max + vector };
}

rect rect_padded(rect* r, const glm::vec2 padding) {
    return { r->min - padding, r->max + padding };
}

rect rect_scaled(rect* r, const glm::vec2 scaling) {
    return { r->min * scaling, r->max * scaling };
}

rect rect_scaled_from_center(rect* r, const glm::vec2 scaling) {
    return rect_from_center(rect_center(r), rect_size(r) * scaling * 0.5f);
}


bool rect_contains(rect* r, const glm::vec2 b) {
    return (b.x >= r->min.x) && (b.y >= r->min.y) && (b.x < r->max.x) && (b.y < r->max.y);
    /*return (Vector<dimensions, T>{b} >= _min).all() &&
        (Vector<dimensions, T>{b} < _max).all();*/
}


bool rect_contains(rect* r, const rect* b) {
    return (b->min.x >= r->min.x) && (b->min.y >= r->min.y) && (b->max.x <= r->max.x) && (b->max.y <= r->max.y);
    /*return (Vector<dimensions, T>{b._min} >= _min).all() &&
        (Vector<dimensions, T>{b._max} <= _max).all();*/
}

float map_range_clamped(const glm::vec2* from, const glm::vec2* to, float alpha) {
    return glm::lerp(to->x, to->y, glm::clamp(lerp_inverted(from->x, from->y, alpha), 0.0f, 1.0f));
}

glm::vec2 map_range_clamped(const glm::vec2* from, const glm::vec2* to, const glm::vec2* alpha) {
    return { 
        glm::lerp(to->x, to->y, glm::clamp(lerp_inverted(from->x, from->y, alpha->x), 0.0f, 1.0f)),
        glm::lerp(to->x, to->y, glm::clamp(lerp_inverted(from->x, from->y, alpha->y), 0.0f, 1.0f)) 
    };
}


glm::vec2 map_range_clamped(const glm::vec2* from_min, const glm::vec2* from_max, const glm::vec2* to_min, const glm::vec2* to_max, const glm::vec2* alpha) {
    return glm::lerp(*to_min, *to_max, glm::clamp(lerp_inverted(*from_min, *from_max, *alpha), 0.0f, 1.0f));
}

rect map_range_clamped(const rect* from, const rect* to, const rect* alpha) {
    return {
        .min = map_range_clamped(&from->min, &from->max, &to->min, &to->max, &alpha->min),
        .max = map_range_clamped(&from->min, &from->max, &to->min, &to->max, &alpha->max)
    };
}

float map_range_unclamped(const glm::vec2* from, const glm::vec2* to, float alpha) {
    return glm::lerp(to->x, to->y, lerp_inverted(from->x, from->y, alpha));
}

glm::vec2 map_range_unclamped(const glm::vec2* from, const glm::vec2* to, const glm::vec2* alpha) {
    return { glm::lerp(to->x, to->y, lerp_inverted(from->x, from->y, alpha->x)) , glm::lerp(to->x, to->y, lerp_inverted(from->x, from->y, alpha->y))};
}

glm::vec2 map_range_unclamped(const glm::vec2* from_min, const glm::vec2* from_max, const glm::vec2* to_min, const glm::vec2* to_max, const glm::vec2* alpha) {
    return glm::lerp(*to_min, *to_max, lerp_inverted(*from_min, *from_max, *alpha));
}

rect map_range_unclamped(const rect* from, const rect* to, const rect* alpha) {
    return {
        .min = map_range_unclamped(&from->min, &from->max, &to->min, &to->max, &alpha->min),
        .max = map_range_unclamped(&from->min, &from->max, &to->min, &to->max, &alpha->max) 
    };
}

}