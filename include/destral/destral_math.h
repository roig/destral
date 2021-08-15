#pragma once
#include <destral/destral_common.h>



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
    using ivec2 = glm::vec<2, ds::i32>;
    using ivec3 = glm::vec<3, ds::i32>;
    using ivec4 = glm::vec<4, ds::i32>;
    using mat3 = glm::mat<3, 3, float>;
   


    //constexpr float pi = 3.14159265f;

    ////// scalar ops
    //inline float min(float a, float b) { return a < b ? a : b; }
    //inline float max(float a, float b) { return b < a ? a : b; }
    //inline float clamp(float a, float lo, float hi) { return max(lo, min(a, hi)); }
    //inline float clamp01(float a) { return max(0.0f, min(a, 1.0f)); }
    //inline float sign(float a) { return a < 0 ? -1.0f : 1.0f; }
    //inline float intersect(float da, float db) { return da / (da - db); }
    //inline float invert_safe(float a) { return a != 0 ? a / 1.0f : 0; }
    //inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
    //inline float remap(float t, float lo, float hi) { return (hi - lo) != 0 ? (t - lo) / (hi - lo) : 0; }
    //inline float mod(float x, float m) { return x - (i32)(x / m) * m; }
    //inline i64 sign(i64 a) { return a < 0 ? -1 : 1; }
    //inline i64 min(i64 a, i64 b) { return a < b ? a : b; }
    //inline i64 max(i64 a, i64 b) { return b < a ? a : b; }
    //inline i64 abs(i64 a) { i64 mask = a >> ((sizeof(i64) * 8) - 1); return (a + mask) ^ mask; }
    //inline i64 clamp(i64 a, i64 lo, i64 hi) { return max(lo, min(a, hi)); }
    //inline i64 clamp01(i64 a) { return max(0, min(a, 1)); }
    //inline bool is_even(i64 x) { return (x % 2) == 0; }
    //inline bool is_odd(i64 x) { return !is_even(x); }

    ////// easing functions
    //inline float smoothstep(float x) { return x * x * (3.0f - 2.0f * x); }
    //inline float ease_out_sin(float x) { return sinf((x * pi) * 0.5f); }
    //inline float ease_in_sin(float x) { return 1.0f - cosf((x * pi) * 0.5f); }
    //inline float ease_in_quart(float x) { return x * x * x * x; }
    //inline float ease_out_quart(float x) { return 1.0f - ease_in_quart(1.0f - x); }
    //
    ////// Remaps the result from atan2f to the continuous range of 0, 2*PI.
    //inline float atan2_360(float y, float x) { return atan2f(-y, -x) + pi; }

    ////// vector 2 ops
    //template <typename T> struct v2t { T x, y; };
    //using iv2 = v2t<i64>;
    //using v2 = v2t<float>;


    //template <typename T> inline v2t<T> add(v2t<T> a, v2t<T> b) { return v2t<T>(a.x + b.x, a.y + b.y); }
    //template <typename T> inline v2t<T> sub(v2t<T> a, v2t<T> b) { return v2t<T>(a.x - b.x, a.y - b.y); }
    ///*inline v2t<T>& operator+=(v2t<T>& a, v2t<T> b) { return a = a + b; }
    //inline v2t<T>& operator-=(v2t<T>& a, v2t<T> b) { return a = a - b; }*/
    //template <typename T> inline float dot(v2t<T> a, v2t<T> b) { return a.x * b.x + a.y * b.y; }
    //template <typename T> inline v2t<T> mul(v2t<T> a, float b) { return v2t<T>(a.x * b, a.y * b); }
    //template <typename T> inline v2t<T> mul(v2t<T> a, v2t<T> b) { return v2t<T>(a.x * b.x, a.y * b.y); }
    ///*inline v2t<T>& operator*=(v2t<T>& a, float b) { return a = a * b; }
    //inline v2t<T>& operator*=(v2t<T>& a, v2t<T> b) { return a = a * b; }*/
    //template <typename T> inline v2t<T> div(v2t<T> a, float b) { return v2t<T>(a.x / b, a.y / b); }
    ///*inline v2t<T>& operator/=(v2t<T>& a, float b) { return a = a / b; }*/
    //template <typename T> inline v2t<T> skew(v2t<T> a) { return v2t<T>(-a.y, a.x); }
    //template <typename T> inline v2t<T> ccw90(v2t<T> a) { return v2t<T>(a.y, -a.x); }
    //template <typename T> inline float det2(v2t<T> a, v2t<T> b) { return a.x * b.y - a.y * b.x; }
    //template <typename T> inline v2t<T> min(v2t<T> a, v2t<T> b) { return v2t<T>(min(a.x, b.x), min(a.y, b.y)); }
    //template <typename T> inline v2t<T> max(v2t<T> a, v2t<T> b) { return v2t<T>(max(a.x, b.x), max(a.y, b.y)); }
    //template <typename T> inline v2t<T> clamp(v2t<T> a, v2t<T> lo, v2t<T> hi) { return max(lo, min(a, hi)); }
    //template <typename T> inline v2t<T> clamp01(v2t<T> a) { return max(v2t<T>(0, 0), min(a, v2t<T>(1, 1))); }
    //template <typename T> inline v2t<T> abs(v2t<T> a) { return v2t<T>(fabsf(a.x), fabsf(a.y)); }
    //template <typename T> inline float hmin(v2t<T> a) { return min(a.x, a.y); }
    //template <typename T> inline float hmax(v2t<T> a) { return max(a.x, a.y); }
    //template <typename T> inline float len(v2t<T> a) { return sqrtf(dot(a, a)); }
    //template <typename T> inline float distance(v2t<T> a, v2t<T> b) { return sqrtf(powf((a.x - b.x), 2) + powf((a.y - b.y), 2)); }
    //template <typename T> inline v2t<T> norm(v2t<T> a) { return a / len(a); }
    //template <typename T> inline v2t<T> safe_norm(v2t<T> a) { float sq = dot(a, a); return sq ? a / sqrtf(sq) : v2t<T>(0, 0); }
    //template <typename T> inline float safe_norm(float a) { return a == 0 ? 0 : sign(a); }
    //template <typename T> inline i64 safe_norm(i64 a) { return a == 0 ? 0 : sign(a); }
    //template <typename T> inline v2t<T> negate(v2t<T> a) { return v2t<T>(-a.x, -a.y); }
    //template <typename T> inline v2t<T> lerp(v2t<T> a, v2t<T> b, float t) { return a + (b - a) * t; }
    //template <typename T> inline v2t<T> bezier(v2t<T> a, v2t<T> c0, v2t<T> b, float t) { return lerp(lerp(a, c0, t), lerp(c0, b, t), t); }
    //template <typename T> inline v2t<T> bezier(v2t<T> a, v2t<T> c0, v2t<T> c1, v2t<T> b, float t) { return bezier(lerp(a, c0, t), lerp(c0, c1, t), lerp(c1, b, t), t); }               
    ////inline i64 operator<(v2t<T> a, v2t<T> b) { return a.x < b.x&& a.y < b.y; }
    ////inline i64 operator>(v2t<T> a, v2t<T> b) { return a.x > b.x && a.y > b.y; }
    ////inline i64 operator<=(v2t<T> a, v2t<T> b) { return a.x <= b.x && a.y <= b.y; }
    ////inline i64 operator>=(v2t<T> a, v2t<T> b) { return a.x >= b.x && a.y >= b.y; }
    //template <typename T> inline v2t<T> floor(v2t<T> a) { return v2t<T>(floorf(a.x), floorf(a.y)); }
    //template <typename T> inline v2t<T> round(v2t<T> a) { return v2t<T>(roundf(a.x), roundf(a.y)); }
    //template <typename T> inline v2t<T> invert_safe(v2t<T> a) { return v2t<T>(invert_safe(a.x), invert_safe(a.y)); }
    //template <typename T> inline i64 parallel(v2t<T> a, v2t<T> b, float tol) {
    //    float k = len(a) / len(b);
    //    b = b * k;
    //    if (fabs(a.x - b.x) < tol && fabs(a.y - b.y) < tol) return 1;
    //    return 0;
    //}

    ////// Computes the shortest angle to rotate the vector a to the vector b.
    //inline float shortest_arc(v2 a, v2 b) {
    //    float c = dot(a, b);
    //    float s = det2(a, b);
    //    float theta = acosf(c);
    //    if (s > 0) {
    //        return theta;
    //    } else {
    //        return -theta;
    //    }
    //}


    //inline float angle_diff(float radians_a, float radians_b) { return mod((radians_b - radians_a) + pi, 2.0f * pi) - pi; }
    //inline v2 v2_from_angle(float radians) { return v2(cosf(radians), sinf(radians)); }



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
    //struct transform {
    //public:
    //    transform(vec2 position = vec2(0), float rotation_rad = 0.0f, vec2 scale = vec2(1)) :
    //        _position(position),
    //        _rot_radians(rotation_rad),
    //        _scale(scale)         {
    //        update_matrix();
    //    }


    //    const mat3& matrix() { return _matrix; }


    //private:
    //    void update_matrix() {
    //        _matrix = glm::scale(glm::rotate(glm::translate(glm::mat3{ 1 }, _position), _rot_radians), _scale);
    //    }

    //    /** Translation of this transformation, as a vector */
    //    vec2 _position = vec2(0);
    //    /** Rotation of this transformation, as a quaternion */
    //    float _rot_radians = 0.0f;
    //    /** 2D scale (always applied in local space) as a vector */
    //    vec2 _scale = vec2(1);
    //    mat3 _matrix;
    //};

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
    *	{min.x,max.y} --------------  max    
    *	    |			               |
    *      min ------------------{max.x,min.y}
    *	
    *
    *      X goes to the right.
    *      Y goes up.
    *
    * NOTE: Implementation details from Magnum Engine
    */
    struct rect {
        vec2 min = { 0,0 }; // bottom left
        vec2 max = { 0,0 }; // top-right

        /**
        * @brief Create a rect from minimal coordinates and size
        * @param min   Minimal coordinates
        * @param size  rect size
        */
        static rect from_size(vec2 min, vec2 size) { return { min, min + size }; }

       // static rect from_top_left_size(vec2 top_left, vec2 size) { return {} }


        /**
         * @brief Create a rect from center and half size
         * @param center    rect center
         * @param halfSize  Half size
         *
         * For creating i32eger center rects you can use @ref from_size()
         * together with @ref padded(), for example:
         */
        static rect from_center(vec2 center, vec2 halfSize) { return { center - halfSize, center + halfSize }; }


        /** @brief bottom left corner */
        vec2 top_left() { return { min.x, max.y }; }
        /** @brief top right corner */
        vec2 bottom_right() { return { max.x, min.y }; }
        /** @brief top left corner equal to min*/
        vec2 bottom_left() { return min; }
        /** @brief bottom right corner equal to max*/
        vec2 top_right() { return max; }

        /** @brief Left edge */
        float left() { return min.x; }
        /** @brief right edge */
        float right() { return max.x; }
        /** @brief top edge */
        float top() { return max.y; }
        /** @brief bottom edge */
        float bottom() { return min.y; }

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
         * @brief Whether given poi32 is contained inside the rect
         *
         * The rect minimum is i32erpreted as inclusive, maximum as exclusive.
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

        static inline mat3 build_matrix(const vec2& translation, float rotation_radians, const vec2& scaling) {
            return glm::scale(glm::rotate(glm::translate(glm::mat3{ 1 }, translation), rotation_radians), scaling);
        }
        static inline mat3 build_matrix(const vec2& translation, float rotation_radians) {
            return glm::rotate(glm::translate(glm::mat3{ 1 }, translation), rotation_radians);
        }
        static inline mat3 build_matrix(const vec2& translation) {
            return glm::translate(glm::mat3{ 1 }, translation);
        }

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