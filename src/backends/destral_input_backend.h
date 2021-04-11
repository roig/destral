#pragma once
#include <destral/core/destral_common.h>
// Idea taken from:
// https://github.com/NoelFB/blah/blob/14a53c0f3a5b68278cc63d7c2769d2e3c750a190/src/internal/graphics_backend.h
// 

namespace ds::graphics_backend {
	void init();
	void shutdown();
	void frame();

}