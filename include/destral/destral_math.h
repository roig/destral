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
    using mat4 = glm::mat<4, 4, float>;


    //constexpr float pi = 3.14159265f;

    //// scalar ops
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
    //inline i32 sign(i32 a) { return a < 0 ? -1 : 1; }
    //inline i32 min(i32 a, i32 b) { return a < b ? a : b; }
    //inline i32 max(i32 a, i32 b) { return b < a ? a : b; }
    //inline i32 abs(i32 a) { i32 mask = a >> ((sizeof(i32) * 8) - 1); return (a + mask) ^ mask; }
    //inline i32 clamp(i32 a, i32 lo, i32 hi) { return max(lo, min(a, hi)); }
    //inline i32 clamp01(i32 a) { return max(0, min(a, 1)); }
    //inline bool is_even(i32 x) { return (x % 2) == 0; }
    //inline bool is_odd(i32 x) { return !is_even(x); }

    //// easing functions
    //inline float smoothstep(float x) { return x * x * (3.0f - 2.0f * x); }
    //inline float ease_out_sin(float x) { return sinf((x * pi) * 0.5f); }
    //inline float ease_in_sin(float x) { return 1.0f - cosf((x * pi) * 0.5f); }
    //inline float ease_in_quart(float x) { return x * x * x * x; }
    //inline float ease_out_quart(float x) { return 1.0f - ease_in_quart(1.0f - x); }
    //

    //// vector ops
    //struct v2 {
    //    float x;
    //    float y;
    //};

    //inline v2 operator+(v2 a, v2 b) { return v2(a.x + b.x, a.y + b.y); }
    //inline v2 operator-(v2 a, v2 b) { return v2(a.x - b.x, a.y - b.y); }
    //inline v2& operator+=(v2& a, v2 b) { return a = a + b; }
    //inline v2& operator-=(v2& a, v2 b) { return a = a - b; }
    //inline float dot(v2 a, v2 b) { return a.x * b.x + a.y * b.y; }
    //inline v2 operator*(v2 a, float b) { return v2(a.x * b, a.y * b); }
    //inline v2 operator*(v2 a, v2 b) { return v2(a.x * b.x, a.y * b.y); }
    //inline v2& operator*=(v2& a, float b) { return a = a * b; }
    //inline v2& operator*=(v2& a, v2 b) { return a = a * b; }
    //inline v2 operator/(v2 a, float b) { return v2(a.x / b, a.y / b); }
    //inline v2& operator/=(v2& a, float b) { return a = a / b; }
    //inline v2 skew(v2 a) { return v2(-a.y, a.x); }
    //inline v2 ccw90(v2 a) { return v2(a.y, -a.x); }
    //inline float det2(v2 a, v2 b) { return a.x * b.y - a.y * b.x; }
    //inline v2 min(v2 a, v2 b) { return v2(min(a.x, b.x), min(a.y, b.y)); }
    //inline v2 max(v2 a, v2 b) { return v2(max(a.x, b.x), max(a.y, b.y)); }
    //inline v2 clamp(v2 a, v2 lo, v2 hi) { return max(lo, min(a, hi)); }
    //inline v2 clamp01(v2 a) { return max(v2(0, 0), min(a, v2(1, 1))); }
    //inline v2 abs(v2 a) { return v2(fabsf(a.x), fabsf(a.y)); }
    //inline float hmin(v2 a) { return min(a.x, a.y); }
    //inline float hmax(v2 a) { return max(a.x, a.y); }
    //inline float len(v2 a) { return sqrtf(dot(a, a)); }
    //inline float distance(v2 a, v2 b) { return sqrtf(powf((a.x - b.x), 2) + powf((a.y - b.y), 2)); }
    //inline v2 norm(v2 a) { return a / len(a); }
    //inline v2 safe_norm(v2 a) { float sq = dot(a, a); return sq ? a / sqrtf(sq) : v2(0, 0); }
    //inline float safe_norm(float a) { return a == 0 ? 0 : sign(a); }
    //inline i32 safe_norm(i32 a) { return a == 0 ? 0 : sign(a); }
    //inline v2 operator-(v2 a) { return v2(-a.x, -a.y); }
    //inline v2 lerp(v2 a, v2 b, float t) { return a + (b - a) * t; }
    //inline v2 bezier(v2 a, v2 c0, v2 b, float t) { return lerp(lerp(a, c0, t), lerp(c0, b, t), t); }
    //inline v2 bezier(v2 a, v2 c0, v2 c1, v2 b, float t) { return bezier(lerp(a, c0, t), lerp(c0, c1, t), lerp(c1, b, t), t); }
    //inline i32 operator<(v2 a, v2 b) { return a.x < b.x&& a.y < b.y; }
    //inline i32 operator>(v2 a, v2 b) { return a.x > b.x && a.y > b.y; }
    //inline i32 operator<=(v2 a, v2 b) { return a.x <= b.x && a.y <= b.y; }
    //inline i32 operator>=(v2 a, v2 b) { return a.x >= b.x && a.y >= b.y; }
    //inline v2 floor(v2 a) { return v2(floorf(a.x), floorf(a.y)); }
    //inline v2 round(v2 a) { return v2(roundf(a.x), roundf(a.y)); }
    //inline v2 invert_safe(v2 a) { return v2(invert_safe(a.x), invert_safe(a.y)); }
    //inline i32 parallel(v2 a, v2 b, float tol) {
    //    float k = len(a) / len(b);
    //    b = b * k;
    //    if (fabs(a.x - b.x) < tol && fabs(a.y - b.y) < tol) return 1;
    //    return 0;
    //}


    //// Remaps the result from atan2f to the continuous range of 0, 2*PI.
    //inline float atan2_360(float y, float x) { return atan2f(-y, -x) + pi; }

    //// Computes the shortest angle to rotate the vector a to the vector b.
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


    //struct m3 {
    //    struct col_type { float x, y, z; };
    //    col_type value[3];
    //};

    //// returns an identity matrix.
    //static m3 identity() {
    //    m3 m;
    //    m.value[0] = m3::col_type(1, 0, 0);
    //    m.value[1] = m3::col_type(0, 1, 0);
    //    m.value[2] = m3::col_type(0, 0, 1);
    //    return m;
    //}

    //inline m3 mul(m3 m1, m3 m2) {
    //    float const SrcA00 = m1.value[0].x;
    //    float const SrcA01 = m1.value[0].y;
    //    float const SrcA02 = m1.value[0].z;
    //    float const SrcA10 = m1.value[1].x;
    //    float const SrcA11 = m1.value[1].y;
    //    float const SrcA12 = m1.value[1].z;
    //    float const SrcA20 = m1.value[2].x;
    //    float const SrcA21 = m1.value[2].y;
    //    float const SrcA22 = m1.value[2].z;
    //        
    //    float const SrcB00 = m2.value[0].x;
    //    float const SrcB01 = m2.value[0].y;
    //    float const SrcB02 = m2.value[0].z;
    //    float const SrcB10 = m2.value[1].x;
    //    float const SrcB11 = m2.value[1].y;
    //    float const SrcB12 = m2.value[1].z;
    //    float const SrcB20 = m2.value[2].x;
    //    float const SrcB21 = m2.value[2].y;
    //    float const SrcB22 = m2.value[2].z;

    //    m3 Result;
    //    Result.value[0].x = SrcA00 * SrcB00 + SrcA10 * SrcB01 + SrcA20 * SrcB02;
    //    Result.value[0].y = SrcA01 * SrcB00 + SrcA11 * SrcB01 + SrcA21 * SrcB02;
    //    Result.value[0].z = SrcA02 * SrcB00 + SrcA12 * SrcB01 + SrcA22 * SrcB02;
    //    Result.value[1].x = SrcA00 * SrcB10 + SrcA10 * SrcB11 + SrcA20 * SrcB12;
    //    Result.value[1].y = SrcA01 * SrcB10 + SrcA11 * SrcB11 + SrcA21 * SrcB12;
    //    Result.value[1].z = SrcA02 * SrcB10 + SrcA12 * SrcB11 + SrcA22 * SrcB12;
    //    Result.value[2].x = SrcA00 * SrcB20 + SrcA10 * SrcB21 + SrcA20 * SrcB22;
    //    Result.value[2].y = SrcA01 * SrcB20 + SrcA11 * SrcB21 + SrcA21 * SrcB22;
    //    Result.value[2].z = SrcA02 * SrcB20 + SrcA12 * SrcB21 + SrcA22 * SrcB22;
    //    return Result;
    //}

    //v2 mul(m3 m1, v2 v1) {

    //}








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