#include "destral/transform.h"
#include "ap_debug.h"

void ap_tr_update_matrices(ecs_world_t* w, ap_cp_transform* tr) {
	AP_ASSERT(w);
	AP_ASSERT(tr);
	
	if (tr->parent) {
		//ap_cp_transform* tr;
		//ecs_co
	}

	//ecs_column()

}

void ap_tr_set_position(ecs_world_t* w, vec2 position) {
	
}

void ap_tr_set_rotation(ecs_world_t* w, float rotation) {

}

void ap_tr_set_scale(ecs_world_t* w, vec2 scale) {

}

void ap_tr_set_parent(ecs_world_t* w, ecs_entity_t to, ecs_entity_t parent) {

}