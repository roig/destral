#pragma once

#include "destral.h"
#include <cstdint>
#include <string>

namespace ds {

	namespace detail {
		struct kv_impl;
	}

	struct kv {  // srlzr
		detail::kv_impl* impl = nullptr;

		~kv();

		enum class op_mode {
			UNITIALIZED = 0,
			WRITE = 1,
			READ = 2,
		};

		// variables
		op_mode mode = op_mode::UNITIALIZED;
		error err;
		///////

		/**
		 * Returns the state
		 */
		op_mode get_mode();

		/**
		 * Parses the text at `data` in a single-pass. Sets the kv to mode READ.
		 */
		error reset_parse(const void* data, std::size_t size);

		/**
		 * Sets the `kv` to write mode `WRITE`, ready to serialize data to `buffer`.
		 */
		void reset_write();



		std::string dump();

		// new implementation
		void key(const char* key);

		error value(std::uint64_t& val);
		error value(std::int64_t& val);
		error value(std::uint32_t& val);
		error value(std::int32_t& val);
		error value(std::uint16_t& val);
		error value(std::int16_t& val);
		error value(std::uint8_t& val);
		error value(std::int8_t& val);
		error value(float& val);
		error value(double& val);
		error value(bool& val);
		error value(std::string& val);


		
		

		// Helper templates
		template <typename T> error key_value(const char* key, T& val) { this->key(key); return value(val); };
		template <typename> error key_value(const char* key, bool& val) { this->key(key); return value(val); };


		error object_begin(const char* key);
		error object_end();

		error array_begin(const char* key, std::size_t& count);
		error array_end();

		error blob(const char* key, std::vector<char>& blob);

	};
}