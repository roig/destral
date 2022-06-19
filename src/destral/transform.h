#pragma once
#include <glm/mat3x3.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>
#include <vector>
#include <functional>

/**
 * transform components and functions
 */

namespace ds::cp {
	struct transform {
		glm::vec2 position = glm::vec2(0);
		glm::vec2 scale = glm::vec2(1);
		float rot_radians = 0.0f;
		glm::mat3 local_to_parent = glm::mat3(1);
		glm::mat3 local_to_world = glm::mat3(1);
		entt::entity parent = entt::null;
		std::vector<entt::entity> children;

	};
}

namespace ds::tr {
	/** sets a new position to the Transform. remember that this will not use colliders.*/
	void set_position(entt::registry& r, entt::entity e, glm::vec2 position);

	/** sets a new rotation to the Transform */
	void set_rotation(entt::registry& r, entt::entity e, float rot_radians);

	/** sets a new scale to the Transform */
	void set_scale(entt::registry& r, entt::entity e, glm::vec2 scale);

	/** sets a new parent to the entity. if parent is entt::null means to remove the parent. */
	void set_parent(entt::registry& r, entt::entity to, entt::entity parent = entt::null);

	/** returns all the children entities from the entity e in the hierarchy */
	std::vector<entt::entity> get_children_hierarchy(entt::registry& r, entt::entity e);

	/** returns all the parents from entity e in the hierarchy */
	std::vector<entt::entity> get_parents_hierarchy(entt::registry& r, entt::entity e);

	/** adds the callbacks for the construction and destruction of the transform component  */
	void set_callbacks(entt::registry& r);
}







