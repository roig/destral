#include "collision.h"
#include "cute_c2.h"
#include "entt/entity/registry.hpp"
#include "transform.h"
#include "ap_debug.h"
#include <algorithm>
#include <optional>


/**
Some instructions from PhysX in order to implement a kinematic controller movement.
Source:
 https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/3.3.4/Manual/BestPractices.html

Example implementations:
 https://github.com/godotengine/godot/blob/00949f0c5fcc6a4f8382a4a97d5591fd9ec380f8/modules/bullet/space_bullet.cpp#L877
 UE Source code.


Basic Algorithm:

    1 Call a SQ-Sweep from the current position of the CCT shape to its goal position.
    2 If no initial overlap is detected, move the CCT shape to the position of the first hit,
    and adjust the trajectory of the CCT by removing the motion relative to the contact normal of the hit.
    3 Repeat Steps 1 and 2 until the goal is reached, or until an SQ-Sweep in Step 1 detects an initial overlap.
    4 If an SQ-Sweep in Step 1 detects an initial overlap, use the GQ Penetration Depth computation
    function to generate a direction for depenetration. Move the CCT shape out of penetration and
    begin again with Step 1.

Here we will try to use the same algorithm but using CuteC2 and in 2D shapes.

TODO LIST:

internal collision system:
- Calculation of the ImpactNormal in the hit_result
- Overlap internal function that returns the overlapping colliders from a collider (here we can use a margin)
- sweep_query function tois against ALL colliders. We don't have a broadphase implemented
only a narrow phase with each collider (it is slower we know that). Maybe some sort of acceleration structure could be done before
doing the toi call. (like AABB overlap checking with a big sweep AABB)

API system functions that this system should have:
- (TODO) Raycasts/LineTrace function to check for hits in a line. (https://docs.unity3d.com/ScriptReference/Physics.Raycast.html)
- (DONE) sweep_multi_collider function to check for overlaps and hits of a collider
- (DONE) move_and_collide/move/move_safe function to sweep a collider. (https://github.com/godotengine/godot/blob/00949f0c5fcc6a4f8382a4a97d5591fd9ec380f8/modules/bullet/space_bullet.cpp#L877)
- (TODO) move function should Trigger overlap and blocking callbacks inside the move function

*/





namespace ds::co {
///////// Collision Globals
// This is the factor used to inflate all the shapes when needed
const float g_collision_inflation = 0.04f;
// This is the depenetration maximum of iterations
const int g_depenetration_max_iter = 100;
// This is the factor that will be multiplied to the normal when depenetration correction
const float g_depenetration_correct_factor = 0.2f;


// Conversion routines to change from Destral glm::vec2 and cute_c2
inline c2v toc2v(const glm::vec2& v) { return { v.x, v.y }; };
inline glm::vec2 to_vec2(const c2v& v) { return { v.x, v.y }; }


// Struct containing the c2 information of a shape ready to be used
struct c2_collider_data {
    union collider_data {
        c2Poly poly;
        c2Circle circle;
    } data;
    C2_TYPE type;
    c2x tr;
};

inline c2_collider_data create_c2_collider(const ds::cp::collider& in, const glm::vec2& position, float radians = 0, float inflation = 0.f) {
    c2_collider_data out;
    if (in.is_rectangle) {
        // when using c2Polygon we use the c2x transform.. 
        //auto rectOffseted = Rectangle::fromCenter({ 0,0 }, in.half_size + glm::vec2(inflation));
        glm::vec2 half_size = in.half_size + glm::vec2(inflation);
        // IMPORTANT NOTE: If we change the layout of .vertx check create_aabb_from_c2_collider and change it there too
        out.data.poly.count = 4;
        out.data.poly.verts[0] = c2V(-half_size.x, half_size.y);
        out.data.poly.verts[1] = c2V(half_size.x, half_size.y);
        out.data.poly.verts[2] = c2V(half_size.x, -half_size.y);
        out.data.poly.verts[3] = c2V(-half_size.x, -half_size.y);

        //out.data.poly.verts[0] = c2V(rectOffseted.topLeft().x, rectOffseted.topLeft().y);
        //out.data.poly.verts[1] = c2V(rectOffseted.topRight().x, rectOffseted.topRight().y);
        //out.data.poly.verts[2] = c2V(rectOffseted.bottomRight().x, rectOffseted.bottomRight().y);
        //out.data.poly.verts[3] = c2V(rectOffseted.bottomLeft().x, rectOffseted.bottomLeft().y);

        // Fill normals (after count and verts)
        c2MakePoly(&out.data.poly);

        out.tr = c2Transform({ position.x, position.y }, radians);
        out.type = C2_TYPE::C2_TYPE_POLY;
    } else {
        // when using circle, we don't use the c2x transform.. 
        out.data.circle = c2Circle{ c2V(position.x,position.y), in.circle_radius + inflation };
        out.tr = c2xIdentity();
        out.type = C2_TYPE::C2_TYPE_CIRCLE;
    }
    return out;
}

inline void update_transform_c2_collider(c2_collider_data& out, const glm::vec2& position, float radians) {
    if (out.type == C2_TYPE::C2_TYPE_POLY) {
        // when using c2Polygon we use the c2x transform.. 
        out.tr = c2Transform({ position.x, position.y }, radians);
    } else {
        // when using circle, we don't use the c2x transform.. 
        out.data.circle = c2Circle{ c2V(position.x,position.y), out.data.circle.r };
        out.tr = c2xIdentity();
    }

}

inline c2AABB create_aabb_from_c2_collider(const c2_collider_data& c) {
    c2AABB aabb;
    if (c.type == C2_TYPE::C2_TYPE_POLY) {
        //IMPORTANT NOTE: We know that for our polys we always have the min value in [3] (bottomLeft)
        aabb.min = c2Mulxv(c.tr, c.data.poly.verts[3]);
        //IMPORTANT NOTE: We know that for our polys we always have the min value in [1] (topRight)
        aabb.max = c2Mulxv(c.tr, c.data.poly.verts[1]);
    } else {


        aabb.min.x = c.data.circle.p.x - c.data.circle.r;
        aabb.min.y = c.data.circle.p.y - c.data.circle.r;
        aabb.max.x = c.data.circle.p.x + c.data.circle.r;
        aabb.max.y = c.data.circle.p.y + c.data.circle.r;
    }
    return aabb;
}



struct ray_result {
    // The entity hit
    entt::entity entity_hit = entt::null;

    // This is the "Time" of impact along the query from [0.0 to 1.0].
    float time = 0.f;

    // Normal of the shape we hit with the ray
    glm::vec2 impact_normal;
};


std::vector<ray_result> cast_ray(entt::registry& r, const glm::vec2& start, const glm::vec2& end, bool ignoreTriggers) {
    std::vector<ray_result> rayHits;
    const auto delta = toc2v(end - start);
    c2Ray ray;
    ray.p = toc2v(start);
    ray.d = c2Norm(delta);
    ray.t = c2Len(delta);
    auto colsView = r.view<ds::cp::transform, ds::cp::collider>();
    for (auto colBEntity : colsView) {
        const auto& colliderB = colsView.get<ds::cp::collider>(colBEntity);
        if (ignoreTriggers && colliderB.is_trigger) {
            continue;
        }

        const auto& trB = colsView.get<ds::cp::transform>(colBEntity);
        c2_collider_data c2colB = create_c2_collider(colliderB, trB.position, trB.rot_radians, 0.f);

        c2Raycast rayOut;
        c2CastRay(ray, &c2colB.data, &c2colB.tr, c2colB.type, &rayOut);

        if (rayOut.t != 1.f) {
            rayHits.push_back({ colBEntity, rayOut.t, to_vec2(rayOut.n) });
        }
    }
    return rayHits;
}

/**
    * @brief This removes the ignored entities from the sweep_results vector. Then sorts the results vector by time starting with 0 time
    */
void sort_by_time_remove_ignored(std::vector<ray_result>& results, const std::vector<entt::entity>& ignored) {
    // Erase all the ignored results
    results.erase(std::remove_if(std::begin(results), std::end(results),
        [&](auto x) {
            return std::find(std::begin(ignored), std::end(ignored), x.entity_hit) != std::end(ignored);
        }
    ), std::end(results));

    // Sort by time
    std::sort(results.begin(), results.end(), [](ray_result& a, ray_result& b) {
        return a.time < b.time;
        });
}


// internal struct for sweep_result results query
struct sweep_result {
    // The entity hit
    entt::entity entity_hit = entt::null;

    // This is the "Time" of impact along the query from [0.0 to 1.0].
    float time = 0.f;
};

/**
* @brief sweeps the ds::cp::collider A against all the ds::cp::collider in the registry.
* This will perform a TOI computation and will return a vector of TOI results that overlap the sweep
* This will also return the overlap with the same collider.
*/
std::vector<sweep_result> sweep_query(entt::registry& r, const ds::cp::collider& colliderA, const glm::vec2& start, const glm::vec2& end, float radians) {
    std::vector<sweep_result> results;
    const auto delta = end - start;
    const c2v deltaA = c2V(delta.x, delta.y);

    // Build c2colA data
    c2_collider_data c2colA = create_c2_collider(colliderA, start, radians, 0.f);

    auto colsView = r.view<ds::cp::transform, ds::cp::collider>();
    for (auto colBEntity : colsView) {
        // Build c2colB data
        const auto& trB = colsView.get<ds::cp::transform>(colBEntity);
        const auto& colliderB = colsView.get<ds::cp::collider>(colBEntity);
        c2_collider_data c2colB = create_c2_collider(colliderB, trB.position, trB.rot_radians, 0.f);

        // Calculate the toi of the colB and colA
        float toi = c2TOI(
            &c2colA.data, c2colA.type, &c2colA.tr, deltaA,
            &c2colB.data, c2colB.type, &c2colB.tr, c2V(0, 0), // B will never move
            1, nullptr
        );

        if (toi != 1.f) {
            results.push_back({});
            sweep_result& result = results.back();
            result.entity_hit = colBEntity;
            result.time = toi;
        }
    }
    return results;
}

/**
* @brief returns true if Layers and Masks are configured for collision
*/
inline bool check_masks_for_collision(const ds::co::layer& layersA, const ds::co::mask& masksA, const ds::co::layer& layersB, const ds::co::mask& masksB) {
    return ((int)layersA & (int)masksB) || ((int)layersB & (int)masksA);
}

/**
* @brief creates and initializes a Hit result if an overlap/hit is found between the rayLayer/rayMask and the other collider
* It will retrieve all the hit_result information needed.
*/
inline std::optional<hit_result> createhit_resultFromray_resultIfCollidesOrOverlaps(entt::registry& r,
    const glm::vec2& start, const glm::vec2& end,
    const ds::co::layer rayLayer, const ds::co::mask rayMask,
    const ray_result& ray_result
) {

    // Is safe to use get, because sweep_query iterated using them
    const auto& colB = r.get<ds::cp::collider>(ray_result.entity_hit);

    // If they can't see each other, no hit result, no overlap
    if (!check_masks_for_collision(rayLayer, rayMask, colB.layers, colB.masks)) {
        return {};
    }

    hit_result hit;
    hit.start = start;
    hit.end = end;
    hit.entity_hit = ray_result.entity_hit;
    hit.time = ray_result.time;
    hit.normal = ray_result.impact_normal;
    hit.impact_point = start + (ray_result.time * (end - start));
    hit.shape_pos = hit.impact_point;

    // Determine if it is a hit or an overlap
    if (colB.is_trigger) {
        hit.blocking = false;
    } else {
        hit.blocking = true;
    }
    return hit;

}



/**
* @brief creates and initializes a Hit result if an overlap/hit is found between shape A and the sweep_result.
* It will retrieve all the hit_result information needed.
*/
inline std::optional<hit_result> createhit_resultFromsweep_resultIfCollidesOrOverlaps(entt::registry& r,
    const glm::vec2& start, const glm::vec2& end, float radians,
    const ds::cp::collider& colA, const sweep_result& sweep_result
) {

    // Is safe to use get, because sweep_query iterated using them
    const auto& colB = r.get<ds::cp::collider>(sweep_result.entity_hit);

    // If they can't see each other, no hit result, no overlap
    if (!check_masks_for_collision(colA.layers, colA.masks, colB.layers, colB.masks)) {
        return {};
    }

    // Create collider for A shape
    const auto toiPositionA = start + (sweep_result.time * (end - start));
    c2_collider_data c2colAToi = create_c2_collider(colA, toiPositionA, radians, 0.f);

    // Now inflate B shape
    const auto& trB = r.get<ds::cp::transform>(sweep_result.entity_hit);
    c2_collider_data c2colBinflated = create_c2_collider(colB, trB.position, trB.rot_radians, g_collision_inflation);

    // Retrieve manifold
    // WARNING! Check c2Manifold to get the normal!! Order is important here!
    c2Manifold manifold;
    c2Collide(
        &c2colBinflated.data, &c2colBinflated.tr, c2colBinflated.type,
        &c2colAToi.data, &c2colAToi.tr, c2colAToi.type,
        &manifold
    );

    if (manifold.count != 0) {
        hit_result hit;
        hit.start = start;
        hit.end = end;
        hit.entity_hit = sweep_result.entity_hit;
        hit.time = sweep_result.time;
        hit.normal = to_vec2(manifold.n);
        hit.impact_point = to_vec2(manifold.contact_points[0]);
        hit.shape_pos = toiPositionA;

        // Determine if it is a hit or an overlap
        if (colA.is_trigger || colB.is_trigger) {
            hit.blocking = false;
        } else {
            hit.blocking = true;
        }
        return hit;
    } else {
        AP_WARNING( "c2Collide can't find a manifold for a previous toi collision!! Skiping this hit result");

        return {};
    }
}

/**
    * @brief This removes the ignored entities from the sweep_results vector. Then sorts the results vector by time starting with 0 time
    */
void sort_by_time_remove_ignored(std::vector<sweep_result>& results, const std::vector<entt::entity>& ignored) {
    // Erase all the ignored results
    results.erase(std::remove_if(std::begin(results), std::end(results),
        [&](auto x) {
            return std::find(std::begin(ignored), std::end(ignored), x.entity_hit) != std::end(ignored);
        }
    ), std::end(results));

    // Sort by time
    std::sort(results.begin(), results.end(), [](sweep_result& a, sweep_result& b) {
        return a.time < b.time;
        });
}

std::vector<hit_result> sweep_multi_collider(entt::registry& r, const ds::cp::collider& collider,
    const glm::vec2& start, const glm::vec2& end, float radians, const std::vector<entt::entity>& ignoredEntities) {

    // First get all the entities that touch the query
    auto sweep_results = sweep_query(r, collider, start, end, radians);
    //gColLog->trace("sweep_multi_collider: sweep_query count: {}", sweep_results.size());

    // Order by time and remove ignored by time!
    sort_by_time_remove_ignored(sweep_results, ignoredEntities);
    //gColLog->trace("sweep_multi_collider: sweep_query count after ignoring: {}", sweep_results.size());

    std::vector<hit_result> hits;
    for (auto& sweep_result : sweep_results) {
        // Create the hit if collides or overlaps
        auto hit = createhit_resultFromsweep_resultIfCollidesOrOverlaps(r, start, end, radians, collider, sweep_result);
        // if a hit or an overlap found
        if (hit) {
            // save it
            hits.push_back(hit.value());
            // if it is a hit, we finished!
            if (hits.back().blocking) {
                return hits;
            }
        }
    }
    // We didn't found any hits, but maybe overlaps, return them
    return hits;
}

std::vector<hit_result> ray_cast(entt::registry& r, const glm::vec2& start, const glm::vec2& end, ds::co::layer layer,
    ds::co::mask mask, const std::vector<entt::entity>& ignoredEntities, bool ignoreTriggers) {
    auto castray_results = cast_ray(r, start, end, ignoreTriggers);
    sort_by_time_remove_ignored(castray_results, ignoredEntities);
    for (const auto& castray_result : castray_results) {
        createhit_resultFromray_resultIfCollidesOrOverlaps(r, start, end, layer, mask, castray_result);
        // TODOOOOOOOOO
        // if a hit or an overlap found
        //if (hit) {
        //    // save it
        //    hits.push_back(hit.value());
        //    // if it is a hit, we finished!
        //    if (hits.back().blocking) {
        //        return hits;
        //    }
        //}
    }
    return {};
}


std::vector<c2Manifold> test_collider(entt::registry& r, const ds::cp::collider& colA, const ds::cp::transform& trA, const std::vector<entt::entity>& ignoredEntities) {
    std::vector<c2Manifold> collisions;

    // Prepare colliderA inflated
    c2_collider_data c2colA = create_c2_collider(colA, trA.position, trA.rot_radians, g_collision_inflation);
    auto colsView = r.view<ds::cp::transform, ds::cp::collider>();
    for (auto colBEntity : colsView) {
        // Check if this entity is ignored or not
        if (std::find(ignoredEntities.begin(), ignoredEntities.end(), colBEntity) != ignoredEntities.end()) {
            continue;
        }

        // If they can't see each other, no overlap
        const auto& colB = colsView.get<ds::cp::collider>(colBEntity);
        if (!check_masks_for_collision(colA.layers, colA.masks, colB.layers, colB.masks)) {
            continue;
        }

        const auto& trB = colsView.get<ds::cp::transform>(colBEntity);
        c2_collider_data c2colB = create_c2_collider(colB, trB.position, trB.rot_radians, 0.0f);


        // First check the AABBs (performance optimization)
        const auto aabbA = create_aabb_from_c2_collider(c2colA);
        const auto aabbB = create_aabb_from_c2_collider(c2colB);
        if (!c2AABBtoAABB(aabbA, aabbB)) {
            continue;
        }

        //// Now check concrete collision
        //if (!c2Collided(&c2colB.data, &c2colB.tr, c2colB.type, &c2colA.data, &c2colA.tr, c2colA.type)) {
        //    continue;
        //}

        // Now that we know we have collided, retrieve manifold

        // WARNING! Check c2Manifold to get the normal!! Order is important here!
        c2Manifold manifold;
        c2Collide(
            &c2colB.data, &c2colB.tr, c2colB.type,
            &c2colA.data, &c2colA.tr, c2colA.type,
            &manifold
        );

        // If collided
        if (manifold.count) {
            collisions.push_back(manifold);
        }
    }
    return collisions;
}


/**
    * @brief This function will try to depenetrate the entity eA.
    * How? It will test_collider against all shapes and for each shape overlapped it will try to depenetrate using the normal.
    *
    */
bool depenetrate(entt::registry& r, entt::entity eA, const ds::cp::collider& colA, const ds::cp::transform& trA) {

    //Make a transform copy to work with it
    ds::cp::transform currTrA = trA;
    int currIt = 0;
    std::vector<c2Manifold> manifoldsFound;
    do {
        manifoldsFound = test_collider(r, colA, currTrA, { {eA} });
        for (auto& m : manifoldsFound) {
            currTrA.position += to_vec2(m.n) * g_depenetration_correct_factor;
        }
        ++currIt;
    } while (currIt < g_depenetration_max_iter && !manifoldsFound.empty());

    // Update the transform after depenetration phase
    tr::set_position(r, eA, currTrA.position);

    if (!manifoldsFound.empty()) {
        AP_WARNING( "Unable to depenetrate entity: %d", entt::to_integral(eA));
        return false;
    } else {
        return true;
    }
}

hit_result move_to(entt::registry& r, entt::entity e, const glm::vec2& position, bool sweep) {
    auto tr = r.try_get<ds::cp::transform>(e);
    // no transform component found, return
    if (!tr) {
        AP_WARNING( "Trying to moveTo an entity without Transform component %d", entt::to_integral(e));
        return {};
    }

    // retrieve delta movement and move
    const auto delta = position - tr->position;
    return move(r, e, delta, sweep);
}


hit_result move(entt::registry& r, entt::entity e, const glm::vec2& delta, bool sweep) {
    // check if delta is nearly zero. Then not move!! Could cause errors inside the sweeps
    const auto len = delta.length();
    if (delta == glm::vec2{0, 0} || (len <= (4.f * 1.e-4f))) {
        //AP_WARNING( "Movement delta too small");
        return {};
    }

    auto trA = r.try_get<ds::cp::transform>(e);
    // no transform component found, return
    if (!trA) {
        return {};
    }

    // Movement without sweeping, update transform and return empty hits.
    if (!sweep) {
        tr::set_position(r, e, trA->position + delta);
        return {};
    }

    ////////////// Movement with sweeping
    // First check if we have a collider or not
    auto colA = r.try_get<ds::cp::collider>(e);
    if (!colA) {
        // If we don't have a collider, just update the transform and return empty hits
        tr::set_position(r, e, trA->position + delta);
        return {};
    }

    //////////////////////////
    // My safemove function that safely moves a collider A consists of:
    //////////////////////////

    // 1 Depenetrate first using c2Collide with collider A inflated.
    depenetrate(r, e, *colA, *trA);

    // 2 we are in a safe position, Sweep to destination (using c2toi) (without inflation)
    auto hits = sweep_multi_collider(r, *colA, trA->position, trA->position + delta, trA->rot_radians, { {e} });
    if (hits.empty()) {
        // If no hits, move all delta!
        tr::set_position(r, e, trA->position + delta);
    } else {
        if (!hits.back().blocking) {
            //Only overlaps, no blocking hits, move all delta!
            tr::set_position(r, e, trA->position + delta);
        } else {
            // we have a hit, so we move only by time
            tr::set_position(r, e, trA->position + (delta * hits.back().time));
        }
    }

    // 3 Now depenetrate againt with collider A inflated.
    depenetrate(r, e, *colA, *trA);


    // 4 Call overlap and hit callback functions if needed
    //TODOOOOOOOOO


    // 5 Returns last hit_result if found
    if (!hits.empty()) {
        return hits.back();
    }
    return {};
}

}