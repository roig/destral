#pragma once


#include <ap_debug.h>
#include <ap_sdl.h>
#include <destral/destral.h>
#include <destral/archtypes.h>
#include <destral/render.h>
#include <destral/ecs.h>
#include <destral/math_funs.h>
using namespace ds;


void ak_tick(entt::registry& r) {

}



void ak_init(entt::registry& r) {
	auto e = r.create();
	r.emplace<cp::transform>(e);
	r.emplace<cp::text_rd>(e);
}


void ak_shutdown(entt::registry& r) {

}

int main() {
	ds::platform_app_desc app_desc;
	app_desc.tick_cb = ak_tick;
	app_desc.init_cb = ak_init;
	app_desc.cleanup_cb = ak_shutdown;
	app_desc.window_name = "Text rendering test";
	app_desc.window_width = 1280;
	app_desc.window_height = 720;
	return ds::app_run(app_desc);
}