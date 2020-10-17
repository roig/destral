#pragma once
#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>
#include <vector>

namespace ds::cp {

	/**
		entity_group makes the entity that has this component to be the root of a set of connected entities.
		- When this component is destroyed, it will destroy all the group entities.
	*/
	struct entity_group {
		std::vector<entt::entity> entities;
	};


}



namespace ds::ecs {
	/** Destroys the entity from the registry. This function will also destroy all the entities in the entity_group if found */
	void destroy(entt::registry& r, entt::entity e);
	void init(entt::registry& r);
	void shutdown();
}