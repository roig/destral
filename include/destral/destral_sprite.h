#pragma once
#include <destral/destral_resource2.h>
#include <destral/destral_image.h>
#include <destral/destral_texture.h>
#include <destral/destral_containers.h>


namespace ds {
	namespace en {
		namespace sprite {
			constexpr const char* name = "ds_sprite_en";
			void register_entity(registry* r);
		}

		namespace sprite_renderer {
			constexpr const char* name = "ds_sprite_renderer_en";
			void register_entity(registry* r);

			// This system renders all the sprite sprite_renderer entities
			void render_sprites(registry* r);

			void update_sprite_animation_frame(registry* r);
		}
	}

	namespace cp {
		struct sprite {
			static constexpr const char* name = "ds_sprite_cp";
			static void register_component(registry* r);

			struct animation {
				struct frame {
					ds::entity texture = entity_null;

					// Time in seconds for this frame
					float time = 0;

					// this is the source rectangle that will be drawn from image in pixels
					rect source_rect_px;
				};

				std::string name;
				//TODO play direction forward.. etc..
				ds::darray<frame> frames;
			};
			ds::dmap<animation> animations;
		};

		struct sprite_renderer {
			static constexpr const char* name = "ds_sprite_renderer_cp";
			static void register_component(registry* r);
			static void serialize(registry* r, ds::entity e, void* cp, bool reading);


			/**
			 * Stops the current animation and sets a new sprite to be played.
			 */
			void set_sprite(entity sprite_e) {
				cur_animation_found = false;
				reset();
				dsverify(r->entity_is_name(sprite_e, en::sprite::name ));
				sprite_to_render = sprite_e;
			}

			/**
			 * Switches to a new aninmation and starts playing it from the beginning.
			 */
			void play(const std::string& animation_name) {
				cur_animation_found = false;
				reset();

				if (!r->entity_valid(sprite_to_render)) {
					return;
				}

				cp::sprite* spr = r->component_try_get<cp::sprite>(sprite_to_render, cp::sprite::name);
				dsverifym(spr, "Entity doesn't contains a sprite component.");
				
				if (spr->animations.empty()) {
					return;
				}

				cur_animation_hash_key = fnv1a_32bit(animation_name);
				auto anim = spr->animations.find(cur_animation_hash_key);
				if (anim) {
					cur_animation_found = true;
				} else {
					DS_WARNING(std::format("Play Sprite animation not found: {}", animation_name));
				}

			}

			/**
			 * Resets the currently playing animation and unpauses the animation.
			 */
			void reset() {
				paused = false;
				cur_frame_idx = 0;
				loop_count = 0;
				t = 0;
			}

			/**
			 * Updates the sprite's internal timer to flip through different frames.
			 */
			void update(float dt) {
				if (paused) return;

				if (!cur_animation_found) return;

				cp::sprite* spr = r->component_try_get<cp::sprite>(sprite_to_render, cp::sprite::name);
				dsverifym(spr, "Entity doesn't contains a sprite component.");

				auto anim = spr->animations.find(cur_animation_hash_key);
				if (!anim) {
					cur_animation_found = false;
				} else {
					t += dt * play_speed_multiplier;
					if (t >= anim->frames[cur_frame_idx].time) {
						cur_frame_idx++;
						if (cur_frame_idx == anim->frames.size()) {
							loop_count++;
							cur_frame_idx = 0;
						}
						DS_LOG(std::format("frame: {}", cur_frame_idx));
						t = 0;

						// TODO - Backwards and pingpong.
					}
				}


				
			}

			entity get_current_texture_entity() {
				if (!cur_animation_found) {
					return entity_null;
				}
				
				cp::sprite* spr = r->component_try_get<cp::sprite>(sprite_to_render, cp::sprite::name);
				dsverifym(spr, "Entity is not an sprite entity");
				auto anim = spr->animations.find(cur_animation_hash_key);
				if (!anim) {
					cur_animation_found = false;
					return entity_null;
				} else {
					entity e_texture = anim->frames[cur_frame_idx].texture;
					dsverifym(r->entity_is_name(e_texture, en::texture::name), "Entity is not a texture entity");
					return e_texture;
				}
			}

			rect get_current_uv_rect() {
				if (!cur_animation_found) {
					return {};
				}
				cp::sprite* spr = r->component_try_get<cp::sprite>(sprite_to_render, cp::sprite::name);
				dsverifym(spr, "Entity is not an sprite entity");
				auto anim = spr->animations.find(cur_animation_hash_key);
				if (!anim) {
					cur_animation_found = false;
					return {};
				} else {
					dsverify(anim->frames.is_valid_index(cur_frame_idx));
					return anim->frames[cur_frame_idx].source_rect_px;
				}

			}

		private:
			entity sprite_to_render = entity_null;
			i32 cur_animation_hash_key;
			bool cur_animation_found = false;
			i32 cur_frame_idx = 0;
			i32 loop_count = 0;
			bool paused = true;
			float t = 0;
			float play_speed_multiplier = 1.0;
			ds::registry* r = nullptr;

		};
	}





}