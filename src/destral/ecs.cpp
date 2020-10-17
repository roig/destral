#include "ecs.h"
#include <entt/entity/registry.hpp>
#include "ap_debug.h"


namespace ds::ecs {

	// callbacks
	namespace cb {

	//void on_entity_group_destruct(entt::registry& r, entt::entity e) {
	//	// dettach all children from this entity
	//	auto& tr = r.get<cp::entity_group>(e);
	//	for (auto child : tr.entities) {
	//		tr::set_parent(r, child, entt::null);
	//	}
	//	// dettach from the parent of this entity
	//	tr::set_parent(r, e, entt::null);
	//}

	}
	void destroy(entt::registry& r, entt::entity e) {
		AP_ASSERT(e != entt::null);

		auto eg = r.try_get<cp::entity_group>(e);
		if (eg) {
			
		}

		r.destroy(e);
	}

	void init(entt::registry& r) {
		
	}

	void shutdown() {

	}
}