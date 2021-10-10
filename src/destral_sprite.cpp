#include <destral/destral_sprite.h>
#include <destral/gfw/destral_gfw.h>
#include <destral/destral_renderer.h>
#include <destral/destral_app.h>

namespace ds {
	namespace cp {
		void sprite::register_component(registry* r) {
			r->component_register<sprite>(sprite::name);
		}

		void sprite_renderer::register_component(registry* r) {
			r->component_register<sprite_renderer>(sprite_renderer::name, sprite_renderer::serialize);

		}

		void sprite_renderer::serialize(registry* r, entity e, void* cp, bool reading) {
			if (reading) {
				cp::sprite_renderer* sr = (cp::sprite_renderer*)cp;
				sr->r = r;
			}
		}

	}

	namespace en {
		namespace sprite {
			void register_entity(registry* r) {
				r->entity_register(en::sprite::name, { cp::sprite::name, cp::resource::name });
			}
		}

		namespace sprite_renderer {
			void register_entity(registry* r) {
				r->entity_register(en::sprite_renderer::name, { cp::sprite_renderer::name, cp::hierarchy::name });
			}

			void update_sprite_animation_frame(registry* r) {
				auto v = r->view_create({ cp::sprite_renderer::name});
				const auto srcp_idx = v.index(cp::sprite_renderer::name);
				while (v.valid()) {
					cp::sprite_renderer* sr = v.data<cp::sprite_renderer>(srcp_idx);
					sr->update(app_dt());
					v.next();
				}
			}

			void render_sprites(registry* r) {
				auto v = r->view_create({ cp::sprite_renderer::name, cp::hierarchy::name});

				const auto hcp_idx = v.index(cp::hierarchy::name);
				const auto srcp_idx = v.index(cp::sprite_renderer::name);
				while (v.valid()) {
					cp::sprite_renderer* sr = v.data<cp::sprite_renderer>(srcp_idx);
					auto texture_e = sr->get_current_texture_entity();
					if (r->entity_valid(texture_e)) {
						dsverify(r->entity_is_name(texture_e, en::texture::name));
						auto texture_cp = r->component_get<cp::texture>(texture_e, cp::texture::name);
						cp::hierarchy* h = v.data<cp::hierarchy>(hcp_idx);
						render_texture(h->ltw(), texture_cp->gpu_texid, { 1, 1 }, sr->get_current_uv_rect() );

						//DS_LOG(std::format("{}", sr->get_current_uv_rect()));
					}
					v.next();
				}
			}
		}
	}
}