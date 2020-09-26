#include "ecs.h"
#include "transform.h"


ecs_entity_t DS_CP_TRANSFORM_ID = 0;

void ds_ecs_register_cp(ecs_world_t* w) {
	//https://stackoverflow.com/questions/1433204/how-do-i-use-extern-to-share-variables-between-source-files
	//https://github.com/SanderMertens/flecs/blob/master/examples/c/07_no_macros/src/main.c
	//https://flecs.docsforge.com/master/api-c/#ecs_add
	/* Register components */
	DS_CP_TRANSFORM_ID = ecs_new_component(w, 0, "Transform", sizeof(ap_cp_transform), ECS_ALIGNOF(ap_cp_transform));
}