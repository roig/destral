#include "rect.h"


namespace ds {



glm::vec2 rect::bottom_left() {
    return { min.x, max.y };
}

glm::vec2 rect::top_right() {
    return { max.x, min.y };
}

glm::vec2 rect::top_left() {
    return min;
}

glm::vec2 rect::bottom_right() {
    return max;
}

float rect::left() {
    return min.x;
}
float rect::right() {
    return max.x;
}
float rect::top() {
    return min.y;
}
float rect::bottom() {
    return max.y;
}


glm::vec2 rect::size() const { return max - min; }

glm::vec2 rect::center() const { return (min + max) / 2.0f; }

rect rect::from_size(const glm::vec2& min, const glm::vec2& size) {
    return { min, min + size };
}
rect rect::from_center(const glm::vec2& center, const glm::vec2& halfSize) {
    return { center - halfSize, center + halfSize };
}

rect rect::translated(const glm::vec2& vector) const {
    return { min + vector, max + vector };
}

rect rect::padded(const glm::vec2& padding) const {
    return { min - padding, max + padding };
}

rect rect::scaled(const glm::vec2& scaling) const {
    return { min * scaling, max * scaling };
}

rect rect::scaled_from_center(const glm::vec2& scaling) const {
    return from_center(center(), size() * scaling * 0.5f);
}


bool rect::contains(const glm::vec2& b) const {
    return (b.x >= min.x) && (b.y >= min.y) && (b.x < max.x) && (b.y < max.y);
    /*return (Vector<dimensions, T>{b} >= _min).all() &&
        (Vector<dimensions, T>{b} < _max).all();*/
}


bool rect::contains(const rect& b) const {
    return (b.min.x >= min.x) && (b.min.y >= min.y) && (b.max.x <= max.x) && (b.max.y <= max.y);
    /*return (Vector<dimensions, T>{b._min} >= _min).all() &&
        (Vector<dimensions, T>{b._max} <= _max).all();*/
}


}