#include "ecs.h"
#include <entt/entity/registry.hpp>
#include "ap_debug.h"


namespace ds::ecs {
	void destroy(entt::registry& r, entt::entity e) {
		AP_ASSERT(e != entt::null);
		auto eg = r.try_get<cp::entity_group>(e);
		if (eg) {
			for (auto grouped_entity : eg->group) {
				r.destroy(grouped_entity);
			}
		}
		r.destroy(e);
	}

	void add_to_group(entt::registry& r, entt::entity group_entity, entt::entity entity_to_add) {
		AP_ASSERT(group_entity != entt::null);
		AP_ASSERT(entity_to_add != entt::null);
		auto& eg = r.get_or_emplace<cp::entity_group>(group_entity);
		eg.group.push_back(entity_to_add);
	}

	void init(entt::registry& r) {
	
	}

	void shutdown() {

	}
}