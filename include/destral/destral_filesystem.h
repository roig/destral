#pragma once
#include <destral/destral_common.h>

namespace ds {
	// Checks whether path refers to existing file system object 
	bool fs_file_exists(const char* file_path);
	// Returns true if the file_path has the extension. NOTE: Extensions are expected with a dot like: ".jpg"
	bool fs_has_extension(const char* file_path, const char* extension);
	// Checks whether file_path refers to existing file system object
	bool fs_is_regular_file(const char* file_path);
}