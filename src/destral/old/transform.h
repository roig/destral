#pragma once
#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>
#include <vector>
#include "destral/types_math.h"

namespace ds::cp {
	struct Transform {
		Vec2 position = Vec2(0.f, 0.f);
		float rotation = 0; // degrees
		Vec2 scale = Vec2(1.0f, 1.0f);
		Mat3 localToWorld;
		Mat3 localToParent;
		entt::entity parent{ entt::null };
		std::vector<entt::entity> children;
	};
}

namespace ds::transform {

}