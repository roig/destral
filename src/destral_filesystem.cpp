#include <destral/destral_filesystem.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace ds {
	bool fs_has_extension(const char* file_path, const char* extension) {
		dsverify(file_path);
		dsverify(extension);
		return fs::path(file_path).extension() == fs::path(extension);
	}

	bool fs_is_regular_file(const char* file_path) {
		dsverify(file_path);
		return fs::is_regular_file(file_path);
	}

	bool fs_file_exists(const char* file_path) {
		dsverify(file_path);
		return fs::exists(file_path);
	}

}