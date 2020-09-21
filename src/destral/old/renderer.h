#pragma once

#include <entt/entity/fwd.hpp>
#include <vector>
#include "destral/types_math.h"
#include "ap_sdl.h"



namespace ds::cp {
	/**
	* @brief Orthographic camera.
	*/
	struct Camera {
		/** The viewport is the rectangle into which the contents of the
		* camera will be displayed, expressed as a factor (between 0 and 1)
		* of the size of the screen window to which the camera is applied.
		* By default, a view has a viewport which covers the entire screen:
		* vec4(0.0, 0.0, 1.0, 1.0)
		*
		* |      *(1,1)
		* |
		* |______
		* (0,0)
		*/
		Vec4 viewport = Vec4(0.f, 0.f, 1.f, 1.f);

		/** @brief color used when the camera clears their viewport */
		Color clear_color = Color(0);

		float aspect_ratio = 16.0f / 9.0f;

		// half vertical size in world units
		float half_vsize = 360.0f;

		/** @brief If true, the camera will be rendered. if false will not be rendered */
		bool is_active = true;
	};

	struct SpriteRenderer {
		// The texture identifier to draw this sprite
		std::string texture_id;
		Color color = Color(1);
	};

	struct CircleRenderer {
		float radius = 10.0;
		Color color = Color::green();
		bool is_filled= false;
	};

	struct LineRenderer {
		// List of Points of the line
		std::vector<Vec2> points;

		// Color of the line
		Color color = Color::green();

		// true if you want to render a line between the last point and the first one (a line loop)
		bool is_looped = false;
	};

	struct RectangleRenderer {
		Vec2 size = Vec2(25.0f, 25.0f);
		Color color = Color::green();
		bool is_filled = true;
	};
}


namespace ds::render {
	// Initializes the renderer
	void init();

	// Destroys all the renderer resources and shutdown the renderer
	void cleanup();

    // Draws all cameras in the registry
    void drawCameras(entt::registry& r, SDL_Window* window);

	//void enableCollidersDebug(entt::registry& r, bool debugEnabled);
	void debugLine(entt::registry& r, const Vec2& start, const Vec2& end, Color color = Color::green(), float duration = 0.0f);
	void debugLines(entt::registry& r, const std::vector<Vec2>& points, bool isLooped = false, Color color = Color::green(), float duration = 0.0f);
	void debugCircle(entt::registry& r, const Vec2& position, float radius = 1.0f, Color color = Color::green(), float duration = 0.0f);


	// Creates a default camera entity (adds a camera and a transform components)
	entt::entity createCameraEntity(entt::registry& r);
}