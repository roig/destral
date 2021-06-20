#pragma once
#include <destral/destral_resource.h>
#include <destral/destral_image.h>

namespace ds {


	struct animation_frame {
		resource<image> image;
		float time = 0;
	};

	struct animation {
		std::string name;
		//TODO play direction forward.. etc..
		std::vector<animation_frame> frames;
	};

	struct animation_collection {
		std::unordered_map<std::string, animation> animations;
	};


	struct sprite {
		/**
		 * Switches to a new aninmation and starts playing it from the beginning.
		 */
		void play(const std::string& animation_name) {
			cur_animation = nullptr;
			if (!animations) {
				reset();
				return;
			}

			auto it = animations->animations.find(animation_name);
			if (it != animations->animations.end()) {
				cur_animation = &it->second;
			} else {
				DS_WARNING(std::format("Sprite animation not found: {}", animation_name));
			}
			reset();
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

			if (!cur_animation) return;

			t += dt * play_speed_multiplier;
			if (t >= cur_animation->frames[cur_frame_idx].time) {
				cur_frame_idx++;
				if (cur_frame_idx == cur_animation->frames.size()) {
					loop_count++;
					cur_frame_idx = 0;
				}

				t = 0;

				// TODO - Backwards and pingpong.
			}
		}

		resource<image> get_current_image() {
			if (!cur_animation) {
				return resource<image>();
			}
			return cur_animation->frames[cur_frame_idx].image;
		}

		resource<animation_collection> animations;
		animation* cur_animation = nullptr;
		i32 cur_frame_idx = 0;
		i32 loop_count = 0;

		bool paused = true;
		float t = 0;
		float play_speed_multiplier = 1.0;
	};
}