#pragma once
#include <destral/destral_kv.h>
#include <destral/thirdparty/json.hpp>

#include <destral/destral_base64.h>

using namespace nlohmann;

namespace ds {
	namespace detail {
		struct kv_impl {

			json base;
			json* current_json; // used for WRITE/READ

			std::vector<const char*> stack_keys; // used for WRITE/READ

			std::vector<json> stack; // used for WRITE only
			std::size_t array_idx = 0;

			const char* curr_key = nullptr;

			kv_impl() {
				base = json::object({});
				stack.clear();
				current_json = &base;
			}
		};
	}

	static inline void s_reset(kv* k) {
		if (k->impl) {
			delete k->impl;
		}
		k->impl = new detail::kv_impl();
		k->err = error::success();
	}

	kv::~kv() {
		if (impl) {
			delete impl;
		}
	}

	kv::op_mode kv::get_mode() {
		return mode;
	}

	error kv::reset_parse(const void* data, std::size_t size) {
		s_reset(this);
		mode = op_mode::READ;

		// no exceptions
		impl->base = json::parse((char*)data, ((char*)data) + size, nullptr, false);
		if (impl->base.is_discarded()) {
			err = error::failure("Unable to parse 'kv' json data");
		}
		return err;
	}

	void kv::reset_write() {
		s_reset(this);
		mode = op_mode::WRITE;
	}


	std::string kv::dump() {
		return impl->base.dump(4);
	}

	template <typename T>
	static inline error s_rw_number_value(kv* k, T& val) {
		if (k->err.is_error) return k->err;
		auto j = k->impl->current_json;

		if (k->mode == kv::op_mode::READ) {
			if (j->is_array()) {
				val = (*j)[k->impl->array_idx];
				k->impl->array_idx++;
			} else {
				auto it = j->find(k->impl->curr_key);
				if (it != j->end()) {
					if (it->is_number()) {
						it->get_to(val);
					} else {
						return error::failure("Key found but it's not a number (Read)");
					}
				} else {
					return error::failure("Unable to find key.");
				}
			}
		} else if (k->mode == kv::op_mode::WRITE) {
			if (j->is_array()) {
				(*j)[k->impl->array_idx] = val;
				k->impl->array_idx++;
			} else {
				(*j)[k->impl->curr_key] = val;
			}
		} else {
			return error::failure("Read or write mode not set.");
		}
		return error::success();
	}

	void kv::key(const char* key) {
		//if (err.is_error) return err;
		impl->curr_key = key;
	}

	error kv::value(std::uint64_t& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(std::int64_t& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(std::uint32_t& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(std::int32_t& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(std::uint16_t& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(std::int16_t& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(std::uint8_t& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(std::int8_t& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(float& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(double& val) {
		return s_rw_number_value(this, val);
	}
	error kv::value(bool& val) {
		if (err.is_error) return err;
		auto j = impl->current_json;
		if (mode == kv::op_mode::READ) {
			auto it = j->find(impl->curr_key);
			if (it != j->end()) {
				if (it->is_boolean()) {
					it->get_to(val);
				} else {
					return error::failure("Key found but it's not a bool (Read)");
				}
			} else {
				return error::failure("Unable to find key.");
			}
		} else if (mode == kv::op_mode::WRITE) {
			(*j)[impl->curr_key] = val;
		} else {
			return error::failure("Read or write mode not set.");
		}
		return error::success();
	}

	error kv::value(std::string& val) {
		if (err.is_error) return err;
		auto j = impl->current_json;
		if (mode == kv::op_mode::READ) {
			auto it = j->find(impl->curr_key);
			if (it != j->end()) {
				if (it->is_string()) {
					it->get_to(val);
				} else {
					return error::failure("Key found but it's not a std::string (Read)");
				}
			} else {
				return error::failure("Unable to find key.");
			}
		} else if (mode == kv::op_mode::WRITE) {
			(*j)[impl->curr_key] = val;
		} else {
			return error::failure("Read or write mode not set.");
		}
		return error::success();
	}



	error kv::blob(const char* key, std::vector<char>& blob) {
		this->key(key);
		if (err.is_error) return err;
		auto j = impl->current_json;
		if (mode == kv::op_mode::READ) {
			auto it = j->find(impl->curr_key);
			if (it != j->end()) {
				if (it->is_string()) {
					std::string encoded_from_json;
					it->get_to(encoded_from_json);
					const auto decoded = ds::base64_decode(encoded_from_json.data());
					blob.assign(decoded.begin(), decoded.end());
				} else {
					return error::failure("Key found but it's not a blob string (Read)");
				}
			} else {
				return error::failure("Unable to find key.");
			}
		} else if (mode == kv::op_mode::WRITE) {
			std::string encoded = ds::base64_encode(blob.data(), blob.size());
			//const std::string encoded = "QkxPQg===";
			encoded.pop_back();
			//encoded.shrink_to_fit();
			(*j)[impl->curr_key] = encoded;
		} else {
			return error::failure("Read or write mode not set.");
		}
		return error::success();
	}


	error kv::object_begin(const char* key) {
		if (err.is_error) return err;

		if (mode == kv::op_mode::READ) {
			auto j = impl->current_json;
			auto it = j->find(key);
			if (it != j->end()) {
				if (it->is_object()) {
					impl->current_json = &(*it);
					impl->stack_keys.push_back(key);
				} else {
					err = error::failure("Key found but it's not an object!");
				}
			} else {
				err = error::failure("Unable to find object key.");
			}
		} else if (mode == kv::op_mode::WRITE) {
			impl->stack_keys.push_back(key);
			impl->stack.push_back(json::object());
			impl->current_json = &impl->stack.back();
		} else {
			err = error::failure("Read or write mode not set.");
		}

		return err;
	}

	error kv::object_end() {
		if (err.is_error) return err;
		
		if (!impl->current_json->is_object()) {
			err = error::failure("Can't end a json object that is not an object.");
			return err;
		}

		if (mode == kv::op_mode::READ) {
			impl->stack_keys.pop_back();
			impl->current_json = &impl->base;
			for (auto& key : impl->stack_keys) {
				impl->current_json = &(*impl->current_json)[key];
			}
		} else if (mode == kv::op_mode::WRITE) {
			// First insert the current object json to the previous json one
			if (impl->stack.size() == 1) {
				impl->base[impl->stack_keys[0]] = impl->stack[0];
			} else {
				(impl->stack[impl->stack.size() - 2])[impl->stack_keys.back()] = impl->stack.back();
			}

			// Pop the stacks
			impl->stack_keys.pop_back();
			impl->stack.pop_back();

			// Retrieve the new current json
			if (impl->stack.empty()) {
				impl->current_json = &impl->base;
			} else {
				impl->current_json = &impl->stack.back();
			}
		} else {
			err = error::failure("Read or write mode not set.");
		}

		return err;

	}

	error kv::array_begin(const char* key, std::size_t& count) {
		if (err.is_error) return err;
		this->key(key);

		if (mode == kv::op_mode::READ) {
			count = 0;
			auto j = impl->current_json;
			auto it = j->find(key);
			if (it != j->end()) {
				if (it->is_array()) {
					impl->current_json = &(*it);
					impl->stack_keys.push_back(key);
					count = impl->current_json->size();
				} else {
					err = error::failure("Key found but it's not an array!");
				}
			} else {
				err = error::failure("Unable to find object key.");
			}
		} else if (mode == kv::op_mode::WRITE) {
			impl->stack_keys.push_back(key);
			impl->stack.push_back(json::array());
			impl->current_json = &impl->stack.back();
		} else {
			err = error::failure("Read or write mode not set.");
		}
		impl->array_idx = 0;
		return err;
	}
	
	error kv::array_end() {
		if (err.is_error) return err;

		if (!impl->current_json->is_array()) {
			err = error::failure("Can't end a json array that is not an array.");
			return err;
		}

		if (mode == kv::op_mode::READ) {
			impl->stack_keys.pop_back();
			impl->current_json = &impl->base;
			for (auto& key : impl->stack_keys) {
				impl->current_json = &(*impl->current_json)[key];
			}
		} else if (mode == kv::op_mode::WRITE) {
			if (impl->stack.size() == 1) {
				impl->base[impl->stack_keys[0]] = impl->stack[0];
			} else {
				//set the last json to the previous one
				(impl->stack[impl->stack.size() - 2])[impl->stack_keys.back()] = impl->stack.back();
			}
			impl->stack_keys.pop_back();
			impl->stack.pop_back();
		} else {
			err = error::failure("Read or write mode not set.");
		}

		return err;
	}

	


}