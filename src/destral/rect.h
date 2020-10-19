#pragma once
#include <glm/vec2.hpp>

namespace ds {

/**
 *	Axis aligned rectangle, minimal coordinate is inclusive, maximal coordinate is exclusive.
 *	    min ------------------{max.x,min.y} 
 *	    |			               |
 *	{min.x,max.y} --------------  max
 *
 *      X goes to the right.
 *      Y goes down. 
 */
struct rect {
    glm::vec2 min = { 0,0 }; // top left
	glm::vec2 max = { 0,0 }; // bottom-right

    /**
    * @brief Create a rect from minimal coordinates and size
    * @param min   Minimal coordinates
    * @param size  rect size
    */
    static rect from_size(const glm::vec2& min, const glm::vec2& size);

    /**
     * @brief Create a rect from center and half size
     * @param center    rect center
     * @param halfSize  Half size
     *
     * For creating integer center rects you can use @ref from_size()
     * together with @ref padded(), for example:
     */
    static rect from_center(const glm::vec2& center, const glm::vec2& halfSize);


    /** @brief bottom left corner */
    glm::vec2 bottom_left();
    /** @brief top right corner */
    glm::vec2 top_right();
    /** @brief top left corner equal to min*/
    glm::vec2 top_left();
    /** @brief bottom right corner equal to max*/
    glm::vec2 bottom_right();

    /** @brief Left edge */
    float left();
    /** @brief right edge */
    float right();
    /** @brief top edge */
    float top();
    /** @brief bottom edge */
    float bottom();

    /** @brief rect size */
    glm::vec2 size() const;

    /** @brief rect center */
    glm::vec2 rect::center() const;

    /**
     * @brief Scaled rect
     *
     * Multiplies the minimal and maximal coordinates by given amount.
     * Center *doesn't* remain the same, use @ref scaled_from_center() for
     * that operation.
     */
    rect scaled(const glm::vec2& scaling) const;
    /**
     * @brief Padded rect
     *
     * Translates the minimal and maximal coordinates by given amount.
     * Center remains the same.
     */
    rect rect::padded(const glm::vec2& padding) const;
    /**
    * @brief Translated rect
    *
    * Translates the minimal and maximal coordinates by given amount. Size
    * remains the same.
    */
    rect rect::translated(const glm::vec2& vector) const;
    /**
     * @brief rect scaled from the center
     *
     * Scales the size, while center remains the same.
     */
    rect rect::scaled_from_center(const glm::vec2& scaling) const;

    /**
     * @brief Whether given point is contained inside the rect
     *
     * The rect minimum is interpreted as inclusive, maximum as exclusive.
     * Results are undefined if the rect has negative size.
     */
    bool contains(const glm::vec2& b) const;

    /**
     * @brief Whether another rect is fully contained inside this rect
     *
     * Returns @cpp true @ce if the following holds for all dimensions
     * Results are undefined if the rect has negative size.
     */
    bool contains(const rect& b) const;

};

}