#include "types.h"


namespace ds {

entt::entity create_camera(entt::registry& r, const glm::vec2& pos, float half_vsize) {
	entt::entity e = r.create();
	r.emplace<cp::transform>(e);
	tr::set_position(r, e, pos);
	auto& c = r.emplace<cp::camera>(e);
	c.half_vsize = half_vsize;
	return e;
}

entt::entity create_line(entt::registry& r, const std::vector<glm::vec2>& points, glm::vec4 color, const glm::vec2& pos) {
	entt::entity e = r.create();
	r.emplace<cp::transform>(e);
	tr::set_position(r, e, pos);
	auto& c = r.emplace<cp::line_rd>(e);
	c.points = points;
	c.color = color;
	return e;
}

entt::entity create_rectangle(entt::registry& r, const glm::vec2& pos, const glm::vec2& size, bool filled, glm::vec4 color) {
	entt::entity e = r.create();
	r.emplace<cp::transform>(e);
	tr::set_position(r, e, pos);
	auto& c = r.emplace<cp::rect_rd>(e);
	c.size = size;
	c.color = color;
	c.filled = filled;
	return e;
}

entt::entity create_sprite(entt::registry& r, sg_image texture, const glm::vec2& pos, float rot, const glm::vec2& scale) {
	entt::entity e = r.create();
	r.emplace<cp::transform>(e);
	tr::set_position(r, e, pos);
	auto& c = r.emplace<cp::sprite_rd>(e);
	c.texture = texture;
	//c.color = glm::vec4{ 1,0,0,1 };
	return e;
}

}