#pragma once
#include <destral/core/destral_common.h>



#include <destral/thirdparty/glm/matrix.hpp>
// Transform thingies
#include <destral/thirdparty/glm/gtx/matrix_transform_2d.hpp>
#include <destral/thirdparty/glm/gtx/transform.hpp>

/**
    2D math functions and structures:
    - vector/matrix types
    - transform type
    - axis aligned rectangle (rect)
    -- Math useful functions --
    - lerp/lerp inverted template.
    - mapping functions
*/

namespace ds {
    using vec2 = glm::vec<2, float>;
    using vec3 = glm::vec<3, float>;
    using vec4 = glm::vec<4, float>;

    using ivec2 = glm::vec<2, ds::i64>;
    using ivec3 = glm::vec<3, ds::i64>;
    using ivec4 = glm::vec<4, ds::i64>;

    using mat3 = glm::mat<3, 3, float>;
    using mat4 = glm::mat<4, 4, float>;

    /**
     * 2D Transform composed of Position, Rotation and Translation.
     *
     * Transforms can be used to convert from one space to another, for example by transforming
     * positions and directions from local space to world space.
     *
     * // TODO::
     * Transformation of position vectors is applied in the order:  Scale -> Rotate -> Translate.
     * Transformation of direction vectors is applied in the order: Scale -> Rotate.
     *
     * Order matters when composing transforms: C = A * B will yield a transform C that logically
     * first applies A then B to any subsequent transformation. Note that this is the opposite order of quaternion (FQuat) multiplication.
     *
     * Example: LocalToWorld = (DeltaRotation * LocalToWorld) will change rotation in local space by DeltaRotation.
     * Example: LocalToWorld = (LocalToWorld * DeltaRotation) will change rotation in world space by DeltaRotation.
     */
     // TODOOOOO
    struct transform {
    public:
        transform(vec2 position = vec2(0), float rotation_rad = 0.0f, vec2 scale = vec2(1)) :
            _position(position),
            _rot_radians(rotation_rad),
            _scale(scale)         {
            update_matrix();
        }


        const mat3& matrix() { return _matrix; }


    private:
        void update_matrix() {
            _matrix = glm::scale(glm::rotate(glm::translate(glm::mat3{ 1 }, _position), _rot_radians), _scale);
        }

        /** Translation of this transformation, as a vector */
        vec2 _position = vec2(0);
        /** Rotation of this transformation, as a quaternion */
        float _rot_radians = 0.0f;
        /** 2D scale (always applied in local space) as a vector */
        vec2 _scale = vec2(1);
        mat3 _matrix;
    };

    //struct scene_cp {
    //public:
    //    /** sets a new position to the Transform. remember that this will not use colliders.*/
    //    glm::vec2 position();
    //    float rotation();
    //    glm::vec2 scale();

    //    /** sets a new position to the Transform. remember that this will not use colliders.*/
    //    void set_position(glm::vec2 position);

    //    /** sets a new rotation to the Transform */
    //    void set_rotation(float rot_radians);

    //    /** sets a new scale to the Transform */
    //    void set_scale(glm::vec2 scale);

    //private:
    //    transform local;
    //    glm::mat3 local_to_parent = glm::mat3(1);
    //    glm::mat3 local_to_world = glm::mat3(1);
    //    entt::entity parent = entt::null;
    //    std::vector<entt::entity> children;

    //};



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
        vec2 min = { 0,0 }; // top left
        vec2 max = { 0,0 }; // bottom-right

        /**
        * @brief Create a rect from minimal coordinates and size
        * @param min   Minimal coordinates
        * @param size  rect size
        */
        static rect from_size(vec2 min, vec2 size) { return { min, min + size }; }

        /**
         * @brief Create a rect from center and half size
         * @param center    rect center
         * @param halfSize  Half size
         *
         * For creating integer center rects you can use @ref from_size()
         * together with @ref padded(), for example:
         */
        static rect from_center(vec2 center, vec2 halfSize) { return { center - halfSize, center + halfSize }; }


        /** @brief bottom left corner */
        vec2 bottom_left() { return { min.x, max.y }; }
        /** @brief top right corner */
        vec2 top_right() { return { max.x, min.y }; }
        /** @brief top left corner equal to min*/
        vec2 top_left() { return min; }
        /** @brief bottom right corner equal to max*/
        vec2 bottom_right() { return max; }

        /** @brief Left edge */
        float left() { return min.x; }
        /** @brief right edge */
        float right() { return max.x; }
        /** @brief top edge */
        float top() { return min.y; }
        /** @brief bottom edge */
        float bottom() { return max.y; }

        /** @brief rect size */
        vec2 size() { return max - min; }

        /** @brief rect center */
        vec2 center() { return (min + max) / 2.0f; }

        /**
         * @brief Scaled rect
         *
         * Multiplies the minimal and maximal coordinates by given amount.
         * Center *doesn't* remain the same, use @ref scaled_from_center() for
         * that operation.
         */
        rect scaled(vec2 scaling) { return { min * scaling, max * scaling }; }

        /**
         * @brief Padded rect
         *
         * Translates the minimal and maximal coordinates by given amount.
         * Center remains the same.
         */
        rect padded(vec2 padding) { return { min - padding, max + padding }; }

        /**
        * @brief Translated rect
        *
        * Translates the minimal and maximal coordinates by given amount. Size
        * remains the same.
        */
        rect translated(vec2 vector) { return { min + vector, max + vector }; }

        /**
         * @brief rect scaled from the center
         *
         * Scales the size, while center remains the same.
         */
        rect scaled_from_center(vec2 scaling) { return from_center(center(), size() * scaling * 0.5f); }

        /**
         * @brief Whether given point is contained inside the rect
         *
         * The rect minimum is interpreted as inclusive, maximum as exclusive.
         * Results are undefined if the rect has negative size.
         */
        bool contains(vec2 b) { return (b.x >= min.x) && (b.y >= min.y) && (b.x < max.x) && (b.y < max.y); }

        /**
         * @brief Whether another rect is fully contained inside this rect
         *
         * Returns true if the following holds for all dimensions
         * Results are undefined if the rect has negative size.
         */
        bool contains(const rect& b) { return (b.min.x >= min.x) && (b.min.y >= min.y) && (b.max.x <= max.x) && (b.max.y <= max.y); }
    };




    namespace math {
        template<typename T>
        inline T lerp(T x, T y, T a) {
            return glm::mix(x, y, a);
        }

        // lerp inverted
        template<class T>
        inline T lerp_inverted(const T a, const T b, const T lerp) {
            return (lerp - a) / (b - a);
        }

        // Map range functions
        inline float map_range_clamped(const vec2& from, const vec2& to, float alpha) {
            return lerp(to.x, to.y, glm::clamp(lerp_inverted(from.x, from.y, alpha), 0.0f, 1.0f));
        }

        inline vec2 map_range_clamped(const vec2& from, const vec2& to, const vec2& alpha) {
            return {
                lerp(to.x, to.y, glm::clamp(lerp_inverted(from.x, from.y, alpha.x), 0.0f, 1.0f)),
                lerp(to.x, to.y, glm::clamp(lerp_inverted(from.x, from.y, alpha.y), 0.0f, 1.0f))
            };
        }


        inline vec2 map_range_clamped(const vec2& from_min, const vec2& from_max, const vec2& to_min, const vec2& to_max, const vec2& alpha) {
            return lerp(to_min, to_max, glm::clamp(lerp_inverted(from_min, from_max, alpha), 0.0f, 1.0f));
        }

        inline rect map_range_clamped(const rect& from, const rect& to, const rect& alpha) {
            return {
                .min = map_range_clamped(from.min, from.max, to.min, to.max, alpha.min),
                .max = map_range_clamped(from.min, from.max, to.min, to.max, alpha.max)
            };
        }

        inline float map_range_unclamped(const vec2& from, const vec2& to, float alpha) {
            return lerp(to.x, to.y, lerp_inverted(from.x, from.y, alpha));
        }

        inline vec2 map_range_unclamped(const vec2& from, const vec2& to, const vec2& alpha) {
            return { lerp(to.x, to.y, lerp_inverted(from.x, from.y, alpha.x)) , lerp(to.x, to.y, lerp_inverted(from.x, from.y, alpha.y)) };
        }

        inline vec2 map_range_unclamped(const vec2& from_min, const vec2& from_max, const vec2& to_min, const vec2& to_max, const vec2& alpha) {
            return lerp(to_min, to_max, lerp_inverted(from_min, from_max, alpha));
        }

        inline rect map_range_unclamped(const rect& from, const rect& to, const rect& alpha) {
            return {
                .min = map_range_unclamped(from.min, from.max, to.min, to.max, alpha.min),
                .max = map_range_unclamped(from.min, from.max, to.min, to.max, alpha.max)
            };
        }
    }





}