#include "transform.h"
#include "ap/ap_debug.h"
#include <entt/entity/registry.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/transform.hpp>
namespace ds {

inline void update_matrices(entt::registry& r, cp::transform& tr) {
	glm::mat3 parent_ltw(1.0f);
	if (tr.parent != entt::null) {
		auto parent_tr = r.try_get<cp::transform>(tr.parent);
		AP_ASSERT(parent_tr);
		parent_ltw = parent_tr->ltw;
	}
	// calculate new local to parent matrix
	tr.ltp = glm::mat3{ 1 };
	tr.ltp = glm::translate(tr.ltp, tr.position);
	tr.ltp = glm::rotate(tr.ltp, tr.rot_radians);
	tr.ltp = glm::scale(tr.ltp, tr.scale);

	tr.ltw = parent_ltw * tr.ltp;
//	const auto ltp = (Magnum::Matrix3::translation(tr.position) * Magnum::Matrix3::rotation(Magnum::Rad(Magnum::Deg(tr.rotation)))) * Magnum::Matrix3::scaling(tr.scale);
}

inline void update_children(entt::registry& r, cp::transform& parent_tr) {
	const auto parent_ltw = parent_tr.ltw;
	for (auto child : parent_tr.children) {
		auto child_tr = r.try_get<cp::transform>(child);
		AP_ASSERT(child_tr);

		// update the new ltp and ltw for that child
		child_tr->ltp = glm::mat3{ 1 };
		child_tr->ltp = glm::translate(child_tr->ltp, child_tr->position);
		child_tr->ltp = glm::rotate(child_tr->ltp, child_tr->rot_radians);
		child_tr->ltp = glm::scale(child_tr->ltp, child_tr->scale);

		//const auto ltp = (Magnum::Matrix3::translation(trchild->position) * Magnum::Matrix3::rotation(Magnum::Rad(Magnum::Deg(trchild->rotation)))) * Magnum::Matrix3::scaling(trchild->scale);
		child_tr->ltw = parent_ltw * child_tr->ltp;
		update_children(r, *child_tr);
	}
}

void on_transform_construct(entt::registry& r, entt::entity e) {
	auto& tr = r.get<cp::transform>(e);
	update_matrices(r, tr);
	update_children(r, tr);
}

void on_transform_destruct(entt::registry& r, entt::entity e) {
	// dettach all children from this entity
	auto& tr = r.get<cp::transform>(e);
	for (auto child : tr.children) {
		tr::set_parent(r, child, entt::null);
	}
	// dettach from the parent of this entity
	tr::set_parent(r, e, entt::null);
}

void tr::set_callbacks(entt::registry& r) {
	r.on_construct<cp::transform>().connect<&on_transform_construct>();
	r.on_destroy<cp::transform>().connect<&on_transform_destruct>();
}

void tr::set_position(entt::registry& r, entt::entity e, glm::vec2 position) {
	auto tr = r.try_get<cp::transform>(e);
	AP_ASSERT(tr);
	tr->position = position;
	update_matrices(r, *tr);
	update_children(r, *tr);
}


void tr::set_rotation(entt::registry& r, entt::entity e, float rot_radians) {
	auto tr = r.try_get<cp::transform>(e);
	AP_ASSERT(tr);
	tr->rot_radians = rot_radians;
	update_matrices(r, *tr);
	update_children(r, *tr);
}


void tr::set_scale(entt::registry& r, entt::entity e, glm::vec2 scale) {
	auto tr = r.try_get<cp::transform>(e);
	AP_ASSERT(tr);
	tr->scale = scale;
	update_matrices(r, *tr);
	update_children(r, *tr);
}


void tr::set_parent(entt::registry& r, entt::entity to, entt::entity new_parent) {
	// check if we have a parent
	auto toTr = r.try_get<cp::transform>(to);
	AP_ASSERT(toTr);

	auto oldParent = toTr->parent;
	if (oldParent != entt::null) {
		// If we are parented, dettach from it

		// remove to entity from oldParent children list
		auto oldParentTr = r.try_get<cp::transform>(oldParent);
		AP_ASSERT(oldParentTr);
		oldParentTr->children.erase(
			std::remove(oldParentTr->children.begin(), oldParentTr->children.end(), to), oldParentTr->children.end());

		// set current parent to entt::null
		toTr->parent = entt::null;

		// update Matrices and Children
		update_matrices(r, *toTr);
		update_children(r, *toTr);
	}

	if (new_parent != entt::null) {
		auto newParentTr = r.try_get<cp::transform>(new_parent);
		AP_ASSERT(newParentTr);

		// Attach to the new parent, add to in the new parent children list
		newParentTr->children.push_back(to);

		// update Matrices and Children
		toTr->parent = new_parent;
		update_matrices(r, *toTr);
		update_children(r, *toTr);
	}
}


std::vector<entt::entity> tr::get_children_hierarchy(entt::registry& r, entt::entity e) {
	auto tr = r.try_get<cp::transform>(e);
	AP_ASSERT(tr);
	std::vector<entt::entity> children_hierarchy = tr->children;
	for (auto child : tr->children) {
		const auto child_children = get_children_hierarchy(r, child);
		children_hierarchy.insert(std::end(children_hierarchy), std::begin(child_children), std::end(child_children));
	}

	return children_hierarchy;
}

std::vector<entt::entity> tr::get_parents_hierarchy(entt::registry& r, entt::entity e) {
	auto tr = r.try_get<cp::transform>(e);
	AP_ASSERT(tr);
	std::vector<entt::entity> parents;
	while (tr && tr->parent != entt::null) {
		parents.push_back(tr->parent);
		tr = r.try_get<cp::transform>(tr->parent);
		AP_ASSERT(tr);
	}
	return parents;
}

}