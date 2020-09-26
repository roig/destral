#pragma once

#include "types_math.h"
#include "ecs.h"

/**
 *
 *
 */

typedef struct ap_cp_transform {
	vec2 position;
	float rotation;
	vec2 scale;
	mat3 ltw;
	mat3 ltp;
	ecs_entity_t parent;
	ecs_entity_t* children_arr; /** children stb array */
}ap_cp_transform;

void ap_tr_set_position(ecs_world_t* w, vec2 position);
void ap_tr_set_rotation(ecs_world_t* w, float rotation);
void ap_tr_set_scale(ecs_world_t* w, vec2 scale);
void ap_tr_set_parent(ecs_world_t* w, ecs_entity_t to, ecs_entity_t parent);






