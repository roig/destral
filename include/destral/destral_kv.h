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
		result err;
		///////

		/**
		 * Returns the state
		 */
		op_mode get_mode();

		/**
		 * Parses the text at `data` in a single-pass. Sets the kv to mode READ.
		 */
		result reset_parse(const void* data, std::size_t size);

		/**
		 * Sets the `kv` to write mode `WRITE`, ready to serialize data to `buffer`.
		 */
		void reset_write();



		std::string dump();

		// new implementation
		void key(const char* key);

		result value(std::uint64_t& val);
		result value(std::int64_t& val);
		result value(std::uint32_t& val);
		result value(std::int32_t& val);
		result value(std::uint16_t& val);
		result value(std::int16_t& val);
		result value(std::uint8_t& val);
		result value(std::int8_t& val);
		result value(float& val);
		result value(double& val);
		result value(bool& val);
		result value(std::string& val);


		
		

		// Helper templates
		template <typename T> result key_value(const char* key, T& val) { this->key(key); return value(val); };
		template <typename> result key_value(const char* key, bool& val) { this->key(key); return value(val); };


		result object_begin(const char* key);
		result object_end();

		result array_begin(const char* key, std::size_t& count);
		result array_end();

		result blob(const char* key, std::vector<char>& blob);

	};
}