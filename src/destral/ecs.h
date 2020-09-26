#pragma once

#include "flecs.h"

extern ecs_entity_t DS_CP_TRANSFORM_ID;

/** Registers all the components of the engine with a world*/
void ds_ecs_register_cp(ecs_world_t* w);