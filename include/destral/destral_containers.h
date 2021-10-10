#pragma once
#include <destral/destral_common.h>
#include <vector>
#include <unordered_map>

namespace ds {
	// Dynamic array
	// Now it uses the std::vector implementation for now
	template <typename T> struct darray {
		darray() = default;
		//darray(darray<T>&& other) { vec = std::move(other); }
		darray(const darray<T>& other) { vec = other.vec; }
		darray(std::initializer_list<T> l): vec(l) {}
		
		///// Insetion
		// Adds the element at the end of the array
		void push_back(const T& Elem) { vec.push_back(Elem); }

		// Removes the last element of the array
		void pop_back() { dsverify(is_valid_index(size()-1)); vec.pop_back(); }

		// Inserts the other array to the end of this one
		void insert(const darray<T>& other) {
			vec.insert(std::end(vec), std::begin(other.vec), std::end(other.vec));
		}

		// Set the number of array elements,
		// If the count number is greater that the current size, it adds copies of elem
		// If the count number is lower that the current size, it pops elements from the array.
		void resize(i32 count, const T& elem) { 
			//vec.resize(count, elem);
			const auto last_size = size();
			if (last_size == count) {
				return;
			} else {
				if (count > last_size) {
					for (i32 i = 0; i < (count - last_size); i++) {
						push_back(elem);
					}
				} else {
					for (i32 i = 0; i < (last_size - count); i++) {
						pop_back();
					}
				}
			}
		}

		///// Removal

		// Removes the element at the index position
		void remove_at(i32 index) {
			dsverify(is_valid_index(index));
			vec.erase(vec.begin() + index);
		}

		// Removes all the elements that are considered equal to the element provided (using operator== function)
		void remove_all(const T& elem) {
			// TODO not optimized.. but it should work
			i32 ele_idx_remove = -1;
			do {
				ele_idx_remove = find(elem);
				if (ele_idx_remove != -1) {
					remove_at(ele_idx_remove);
				}
			} while (ele_idx_remove != -1);
		}

		void remove_single(const T& elem) {
			i32 ele_idx_remove = find(elem);
			if (ele_idx_remove != -1) {
				remove_at(ele_idx_remove);
			}
		}

		// Removes the element at the index position by swapping it with the element of the last position (Faster, Order NOT PRESERVED)
		void remove_swap_at(i32 index) {
			dsverify(is_valid_index(index));
			if (size() == 1) {
				vec.pop_back();
			} else {
				std::swap(vec[index], vec[size() - 1]);
				vec.pop_back();
			}
		}

		// Removes all the elements of the array
		void clear() {	vec.clear(); }

		///// Iteration/ Queries

		// Returns true only if the index is a valid array index.
		bool is_valid_index(i32 index) const { return index >= 0 && index < checked_size(); }

		// Returns how many elements it holds
		i32 size() const { return checked_size(); }

		// Returns true only if the array is empty
		bool empty() const { return vec.empty(); }

		// Direct access to the array memory (C-Style API), pointer valid as long as
		// the array exists and before any mutating operation. Only the first .size() indices are dereferenceable
		T* data() { vec.data(); }


		T& operator[] (i32 index) {
			dsverify(is_valid_index(index));
			return vec[index];
		}

		const T& operator[] (i32 index) const {
			dsverify(is_valid_index(index));
			return vec[index];
		}

		// Returns the last element of the array
		T& back() { return this->operator[](size() - 1); }

		// Returns true only if the element was found
		bool contains(const T& other) const {
			for (i32 i = 0; i < size(); i++) {
				if (vec[i] == other) {
					return true;
				}
			}
			return false;
		}

		// Returns the index of the first element found or -1 if it's not found
		i32 find(const T& other) const {
			for (i32 i = 0; i < size(); i++) {
				if (vec[i] == other) { 
					return i; 
				}
			}
			return -1;
		}

		// Returns the index of the last element found or -1 if it's not found
		i32 find_last(const T& other) const {
			i32 last_index = -1;
			for (i32 i = 0; i < size(); i++) {
				if (vec[i] == other) { 
					last_index = i;
				}
			}
			return last_index;
		}

		// Returns the index if the element is found by the predicate or -1 if not found
		i32 find_predicate(bool (*pred_fn)(T*)) const {
			dsverify(pred_fn);
			for (i32 i = 0; i < size(); i++) {
				if (pred_fn(vec[i])) {
					return i;
				}
			}
			return -1;
		}

		///// Sorting TODO

	private:
		i32 checked_size() const {
			dsverify(vec.size() < (size_t)std::numeric_limits<i32>::max());
			return (i32)vec.size();
		}
		std::vector<T> vec;
	};




    template<typename T> struct dmap {

        dmap() = default;
        dmap(const dmap<T>& other) { map = other.map; }

        // Returns how many elements it holds
        i32 size() const { return checked_size(); }

        // Returns true only if the array is empty
        bool empty() const { return map.empty(); }

        // Removes all the elements of the array
        void clear() {	map.clear(); }

        // Returns true only if the key is a valid array index.
        bool is_valid_key(i32 key) const { return map.contains(key); }

        // Returns true only if the element was found
        bool contains(const T& other) const {
            for (auto pair : map) {
                if (pair.second == other) {
                    return true;
                }
            }
            return false;
        }

		// Returns a valid pointer if the element with the key was found.
		// Returns nullptr if the element with the key was not found
		T* find(i32 key) {
			auto search = map.find(key);
			if (search != map.end()) {
				return &search->second;
			} else {
				return nullptr;
			}
		}

        // Returns true only if the element was found.
        // If the element is found, it sets the key parameter to the key of the element found.
        // If not the key parameter is unmodified.
        bool find(const T& other, i32& key) const {
            for (auto pair : map) {
                if (pair.second == other) {
                    key = pair.first;
                    return true;
                }
            }
            return false;
        }

        // Returns the number of equal elements found. 0 if no elements were found.
        // When an element is found it fills the pairs array with a pair<key,element> for each one.
        i32 find_all(const T& other, darray<std::pair<i32,T>>& pairs) const {
            i32 found = 0;
            for (auto pair : map) {
                if (pair.second == other) {
                    pairs.push_back(std::make_pair(pair.first,pair.second));
                    found++;
                }
            }
            return found;
        }

        void set(i32 key, const T& elem) {
            map[key] = elem;
        }

        void set(dmap<T>& other_map) {
            for (auto pair : other_map) {
                map[pair.first] = pair.second;
            }
        }


      
        darray<T> to_array() const{
            darray<T> array;
            for (auto pair : map) {
                array.push_back(pair.second);
            }
            return array;
        }

        // Removes the element at the index position
        void remove(i32 key) {
            dsverify(is_valid_key(key));
            map.erase(key);
        }

        void remove(const T& elem) {
            i32 ele_key_remove = -1;
            if (find(elem, ele_key_remove)) {
                remove(ele_key_remove);
            }
        }

        // Removes all the elements that are considered equal to the element provided (using operator== function)
        void remove_all(const T& elem) {
            darray<std::pair<i32,T>> pairs_found;
            find_all(elem,pairs_found);
            for (auto pair : pairs_found) {
                remove(pair.first);
            }
        }


    private:
        i32 checked_size() const {
            dsverify(map.size() < (size_t)std::numeric_limits<i32>::max());
            return (i32)map.size();
        }
        std::unordered_map<i32,T> map;
    };

}




/**
namespace std {

	template<> struct hash<ds::darray<ds::i32>> {
		std::size_t operator()(const ds::darray<ds::i32> & Array) const {
			size_t Seed = 0;
			for (const auto& Elem : Array)
				Seed ^= std::hash<int>()(Elem) + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
			return Seed;
		}
	};
}
*/
