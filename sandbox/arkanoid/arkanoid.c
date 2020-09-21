#pragma once
#include <destral/destral.h>




//
void example_tick(void *r) {
	
}

void example_init(void *r) {
	
}



int main() {
	struct ds_app_desc app_desc = {0};

	app_desc.frame_cb = example_tick;
	app_desc.init_cb = example_init;
	app_desc.window_name = "Arkanoid Game";
	app_desc.window_width = 640;
	app_desc.window_height = 360;

	return ds_app_run(&app_desc);
}
