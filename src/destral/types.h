#pragma once

#include <entt/entity/registry.hpp>
#include "transform.h"
#include "render.h"



namespace ds {
	entt::entity create_camera(entt::registry& r, const glm::vec2& pos = {}, float half_vsize = 360.f);
	entt::entity create_line(entt::registry& r, const std::vector<glm::vec2>& points = {}, glm::vec4 color = { 0,1,0,1 }, const glm::vec2& pos = {});
	entt::entity create_rectangle(entt::registry& r, const glm::vec2& pos, const glm::vec2& size, bool filled = false, glm::vec4 color = { 0,1,0,1 });
	entt::entity create_sprite(entt::registry& r, sg_image texture, const glm::vec2& pos = {}, float rot = 0, const glm::vec2& scale = { 1,1 });
}