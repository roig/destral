#pragma once

#include <destral/destral_math.h>
#include <destral/destral_ecs.h>
#include <destral/destral_containers.h>

// game framework
namespace ds {
	namespace en {
		namespace camera {
			constexpr const char* name = "ds_camera_entity";
			void register_entity(registry* r);
			// Renders all the camera entities
			void render_cameras_system(registry* r);
		}
	}


	namespace cp {
		// This is the struct that handles the hierarchy of entities.
		struct hierarchy {
		public:
			static constexpr const char* name = "hierarchy";
			static void register_component(registry* r);

			const vec2& pos() { return _pos; }
			const vec2& scale() { return _scale; }
			float rot() { return _rot_degrees; }
			entity parent() { return _parent; }
			const darray<entity>& children() { return _children; }
			const mat3& ltw() { return _ltw; }
			const mat3& ltp() { return _ltp; }

			// Sets a new position. This will update children. Remember that this will not use colliders.
			void set_pos(const vec2& new_pos) {
				_pos = new_pos;
				update_matrices();
				update_children(*this);
			}

			// Sets a new scale. This will update children. Remember that this will not use colliders.
			void set_scale(const vec2& new_scale) {
				_scale = new_scale;
				update_matrices();
				update_children(*this);

			}

			// Sets a new rotation degrees. This will update children. Remember that this will not use colliders.
			void set_rot(float new_rot_degrees) {
				_rot_degrees = new_rot_degrees;
				update_matrices();
				update_children(*this);
			}

			// sets a new parent to the entity. if parent is entt::null means to remove the parent.
			void set_parent(entity new_parent) {
				entity to = _entity;

				// check if we have a parent
				auto oldParent = _parent;
				if (oldParent != entity_null) {
					// If we are parented, dettach from it
					// remove to entity from oldParent children list
					auto oldParent_tr = _registry->component_try_get<hierarchy>(oldParent, name);
					dsverify(oldParent_tr);

					oldParent_tr->_children.remove_single(to);

					// set current parent to entt::null
					_parent = entity_null;

					// update Matrices and Children
					update_matrices();
					update_children(*this);
				}

				if (new_parent != entity_null) {
					auto newParentTr = _registry->component_try_get<hierarchy>(new_parent, name);
					dsverify(newParentTr);

					// Attach to the new parent, add to in the new parent children list
					newParentTr->_children.push_back(to);

					// update Matrices and Children
					_parent = new_parent;
					update_matrices();
					update_children(*this);
				}
			};

			void add_child(entity new_child) {
				auto child_hr = _registry->component_try_get<hierarchy>(new_child, name);
				dsverify(child_hr);
				child_hr->set_parent(_entity);
			}

			void add_children(const ds::darray<entity>& new_children) {
				for (i32 i = 0; i < new_children.size(); i++) { add_child(new_children[i]); }
			}

			void remove_child(entity child_to_remove) {
				auto child_hr = _registry->component_try_get<hierarchy>(child_to_remove, name);
				dsverify(child_hr);
				child_hr->set_parent(entity_null);
			}

			void remove_children(const ds::darray<entity>& children_to_remove) {
				for (i32 i = 0; i < children_to_remove.size(); i++) { remove_child(children_to_remove[i]); }
			}

			// returns all the children entities from the entity e in the hierarchy 
			darray<entity> get_children_hierarchy() {
				darray<entity> children_hierarchy;
				children_hierarchy.insert(_children);
				for (i32 i = 0; i < _children.size(); i++) {
					auto child_hr = _registry->component_try_get<hierarchy>(_children[i], name);
					dsverify(child_hr);
					children_hierarchy.insert(child_hr->get_children_hierarchy());
				}
				return children_hierarchy;
			}

			// returns all the parents from entity e in the hierarchy
			darray<entity> get_parents_hierarchy() {
				darray<entity> parents;
				hierarchy* hr = this;
				while (hr && (hr->_parent != entity_null)) {
					parents.push_back(hr->_parent);
					hr = _registry->component_try_get<hierarchy>(hr->_parent, name);
				}
				return parents;
			}

			// Component serialize and cleanup
			static void serialize(registry* r, entity e, void* cp, bool reading) {
				if (reading) {
					hierarchy* h = (hierarchy*)cp;
					h->_registry = r;
					h->_entity = e;
				}
			}

			static void cleanup(registry* r, entity e, void* cp) {
				hierarchy* h = (hierarchy*)cp;
				// dettach all children from this entity
				for (i32 i = 0; i < h->_children.size(); i++) {
					auto child_hr = h->_registry->component_try_get<hierarchy>(h->_children[i], name);
					dsverify(child_hr);
					child_hr->set_parent(entity_null);
				}
				// dettach us from the parent of this entity
				h->set_parent(entity_null);
			}

		private:
			vec2 _pos = { 0, 0 };
			vec2 _scale = { 1, 1 };
			float _rot_degrees = 0;
			mat3 _ltp = glm::mat3(1);
			mat3 _ltw = glm::mat3(1);
			entity _parent = entity_null;
			darray<entity> _children;
			registry* _registry = nullptr;
			entity _entity = entity_null; // Entity that holds this hierarchy component

			inline void update_matrices() {
				glm::mat3 parent_ltw(1.0f);
				if (_parent != entity_null) {
					auto parent_tr = _registry->component_try_get<hierarchy>(_parent, name);
					dsverify(parent_tr);
					parent_ltw = parent_tr->_ltw;
				}
				// calculate new local to parent matrix
				_ltp = glm::mat3{ 1 };
				_ltp = glm::translate(_ltp, _pos);
				_ltp = glm::rotate(_ltp, glm::radians(_rot_degrees));
				_ltp = glm::scale(_ltp, _scale);

				// update the local to world matrix with the parent local to world matrix
				_ltw = parent_ltw * _ltp;
			}

			inline void update_children(hierarchy& parent_tr) {
				const auto parent_ltw = parent_tr._ltw;
				const auto& parent_children = parent_tr.children();
				for (i32 i = 0; i < parent_children.size(); i++) {
					auto child = parent_children[i];
					auto child_tr = _registry->component_try_get<hierarchy>(child, name);
					dsverify(child_tr);

					// update the new local_to_parent and local_to_world for that child
					child_tr->_ltp = glm::mat3{ 1 };
					child_tr->_ltp = glm::translate(child_tr->_ltp, child_tr->_pos);
					child_tr->_ltp = glm::rotate(child_tr->_ltp, glm::radians(child_tr->_rot_degrees));
					child_tr->_ltp = glm::scale(child_tr->_ltp, child_tr->_scale);

					//const auto local_to_parent = (Magnum::Matrix3::translation(trchild->position) * Magnum::Matrix3::rotation(Magnum::Rad(Magnum::Deg(trchild->rotation)))) * Magnum::Matrix3::scaling(trchild->scale);
					child_tr->_ltw = parent_ltw * child_tr->_ltp;
					update_children(*child_tr);
				}
			}
		};

		// Orthographic camera.
		struct camera {
			static constexpr const char* name = "ds_camera_cp";
			static void register_component(registry* r);

			// The viewport is the rectangle into which the contents of the
			// camera will be displayed, expressed as a factor (between 0 and 1)
			// of the size of the screen window to which the camera is applied.
			// By default, a view has a viewport which covers the entire screen:
			// vec4(0.0, 0.0, 1.0, 1.0)
			// 
			// |      *(1,1)
			// |
			// |______
			// (0,0)
			// 
			glm::vec4 viewport = glm::vec4(0.f, 0.f, 1.f, 1.f);

			/** @brief color used when the camera clears their viewport */
			glm::vec4 clear_color = glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f };

			// ortho width size in world units
			float ortho_width = 1.0f;

			// aspect ratio of the camera, used to calculate the height, based on ortho_width
			float aspect = 16.0f / 9.0f;


		};
	}
	
	

}