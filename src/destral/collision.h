#pragma once
#include <glm/vec2.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>

/**
 * The collision system knows how to move entities that have a collider component.
 * You must use those system functions in order to move entities that have a collider.

 * WARNING: The collision functions that sweeps will not work when rotating an entity.
 * You can move an entity that is rotated but you can't sweep a rotation. This is a known limitation of cute_c2 library
 */

namespace ds::co {

    enum class mask : int {
        mask_0 = 1 << 0,
        mask_1 = 1 << 1,
        mask_2 = 1 << 2,
        mask_3 = 1 << 3,
        mask_4 = 1 << 4,
        mask_5 = 1 << 5,
        mask_6 = 1 << 6,
        mask_7 = 1 << 7,
    };

    enum class layer: int {
        layer_0 = 1 << 0,
        layer_1 = 1 << 1,
        layer_2 = 1 << 2,
        layer_3 = 1 << 3,
        layer_4 = 1 << 4,
        layer_5 = 1 << 5,
        layer_6 = 1 << 6,
        layer_7 = 1 << 7,
    };

    struct hit_result {
        // Whether or not the hit was a blocking hit.
        // Due to the ability to have traces simply overlap and not stop the trace.
        bool blocking = false;

        // This is the "Time" of impact along the trace direction ranging from [0.0 to 1.0]. If there was no hit, this will return 1.0.
        float time = 0.f;

        // The world space location of the hit that is modified based on the shape of the trace.
        glm::vec2 shape_pos;

        // The absolute location of the hit. Does not include the shape of the trace, only the point of the hit.
        glm::vec2 impact_point;

        // Normal for the object we moved. (normally is the normal calculated from the impactPoint to the center of the collider)
        glm::vec2 normal;

        // Normal of the collider we hit (normally is the normal of the edge) (TODO)
        // Vec2 impact_normal;

        // The entity hit/overlaped by the trace
        entt::entity entity_hit = entt::null;

        // Start position of the trace
        glm::vec2 start;

        // End position of the trace
        glm::vec2 end;

    };
}

namespace ds::cp {
    /**
     * Collider component
     */
    struct collider {
        // Decides if this collider is a rectangle or a circle
        bool is_rectangle = true;

        // Rectangle extents (only used if isRectangle is true)
        glm::vec2 half_size;

        // Circle radius (only used if isRectangle is false)
        float circle_radius = 10.0f;

        // Layers Mask that the entity appears in. By default, Layer_0.
        ds::co::layer layers = ds::co::layer::layer_0;

        // Layers Mask the entity will scan for collisions. If no mask set will be ignored. By default, Mask_0
        ds::co::mask masks = ds::co::mask::mask_0;

        // If true, this collider will be only overlaped
        bool is_trigger = false;
    };
}

namespace ds::co {
    /**
     * @brief Moves an entity.
     * In order to move an entity, it must have a transform component and an optional collider component.
     * If the entity doesn't have a Collider component we teleport the entity directly modifying the Transform component. No collision checks made
     * If the sweep flag is false, we teleport the entity to the end location, position + movement. No collision checks made.
     * If the sweep flag is true, we move sweeping till we find a blocking hit. Collision and overlap checks done.
     * Only if the sweep flag is true, we will signal overlap and hit callbacks we found. (TODO)
     *
     *
     * @param registry
     * @param entity
     * @param delta
     * @param sweep
     * @return hit_result of the last hit if any and if sweep is true.
     */
    hit_result move(entt::registry& registry, entt::entity entity, const glm::vec2& delta, bool sweep = true);
    hit_result move_to(entt::registry& registry, entt::entity entity, const glm::vec2& position, bool sweep = true);


    /**
    * @brief Trace a collider against the all the colliders in the registry and return overlapping hits and
    * then the first blocking hit if it exists.
    * Results are sorted, so a blocking hit (if found) will be the last element of the array
    * Only the single closest blocking result will be generated, no more results will be returned after that.
    *
    * @param registry
    * @param collider
    * @param start
    * @param end
    * @param rotation
    * @param ignoredEntities
    * @return std::vector<HitResult>
   */
    //std::vector<hit_result> sweep_multi(entt::registry& registry, const cp::collider& collider, const glm::vec2& start, const glm::vec2& end, float rotation_rad, const std::vector<entt::entity>& ignoredEntities = {});

}