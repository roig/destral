#pragma once
#include <glm/vec2.hpp>

/*
    2d math functions and structures:

    - axis aligned rectangle (rect)
    - mapping functions
    - lerp inverted template.


*/

namespace ds {
    /**
    *	Axis aligned rectangle, minimal coordinate is inclusive, maximal coordinate is exclusive.
    *	    min ------------------{max.x,min.y}
    *	    |			               |
    *	{min.x,max.y} --------------  max
    *
    *      X goes to the right.
    *      Y goes down.
    *
    * NOTE: Implementation details from Magnum Engine
    */
    struct rect {
        glm::vec2 min = { 0,0 }; // top left
        glm::vec2 max = { 0,0 }; // bottom-right
    };

    /**
    * @brief Create a rect from minimal coordinates and size
    * @param min   Minimal coordinates
    * @param size  rect size
    */
    rect rect_from_size(glm::vec2 min, glm::vec2 size);

    /**
     * @brief Create a rect from center and half size
     * @param center    rect center
     * @param halfSize  Half size
     *
     * For creating integer center rects you can use @ref from_size()
     * together with @ref padded(), for example:
     */
    rect rect_from_center(glm::vec2 center, glm::vec2 halfSize);


    /** @brief bottom left corner */
    glm::vec2 rect_bottom_left(rect* r);
    /** @brief top right corner */
    glm::vec2 rect_top_right(rect* r);
    /** @brief top left corner equal to min*/
    glm::vec2 rect_top_left(rect* r);
    /** @brief bottom right corner equal to max*/
    glm::vec2 rect_bottom_right(rect* r);

    /** @brief Left edge */
    float rect_left(rect* r);
    /** @brief right edge */
    float rect_right(rect* r);
    /** @brief top edge */
    float rect_top(rect* r);
    /** @brief bottom edge */
    float rect_bottom(rect* r);

    /** @brief rect size */
    glm::vec2 rect_size(rect* r);

    /** @brief rect center */
    glm::vec2 rect_center(rect* r);

    /**
     * @brief Scaled rect
     *
     * Multiplies the minimal and maximal coordinates by given amount.
     * Center *doesn't* remain the same, use @ref scaled_from_center() for
     * that operation.
     */
    rect rect_scaled(rect* r, glm::vec2 scaling);
    /**
     * @brief Padded rect
     *
     * Translates the minimal and maximal coordinates by given amount.
     * Center remains the same.
     */
    rect rect_padded(rect* r, glm::vec2 padding);
    /**
    * @brief Translated rect
    *
    * Translates the minimal and maximal coordinates by given amount. Size
    * remains the same.
    */
    rect rect_translated(rect* r, glm::vec2 vector);
    /**
     * @brief rect scaled from the center
     *
     * Scales the size, while center remains the same.
     */
    rect rect_scaled_from_center(rect* r, glm::vec2 scaling);

    /**
     * @brief Whether given point is contained inside the rect
     *
     * The rect minimum is interpreted as inclusive, maximum as exclusive.
     * Results are undefined if the rect has negative size.
     */
    bool rect_contains(rect* r, glm::vec2 b);

    /**
     * @brief Whether another rect is fully contained inside this rect
     *
     * Returns true if the following holds for all dimensions
     * Results are undefined if the rect has negative size.
     */
    bool rect_contains(rect* r, const rect* b);



    // lerp inverted

	template<class T>
	inline T lerp_inverted(const T a, const T b, const T lerp) {
		return (lerp - a) / (b - a);
	}


    ////// Map range functions
	float map_range_clamped(const glm::vec2* from, const glm::vec2* to, float alpha);
	glm::vec2 map_range_clamped(const glm::vec2* from, const glm::vec2* to, const glm::vec2* alpha);
	glm::vec2 map_range_clamped(const glm::vec2* from_min, const glm::vec2* from_max, const glm::vec2* to_min, const glm::vec2* to_max, const glm::vec2* alpha);
	rect map_range_clamped(const rect* from, const rect* to, const rect* alpha);

	float map_range_unclamped(const glm::vec2* from, const glm::vec2* to, float alpha);
	glm::vec2 map_range_unclamped(const glm::vec2* from, const glm::vec2* to, const glm::vec2* alpha);
	glm::vec2 map_range_unclamped(const glm::vec2* from_min, const glm::vec2* from_max, const glm::vec2* to_min, const glm::vec2* to_max, const glm::vec2* alpha);
	rect map_range_unclamped(const rect* from, const rect* to, const rect* alpha);

}