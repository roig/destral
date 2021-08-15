#include <destral/destral_common.h>
#include <vector>

namespace ds {


	//namespace vector {
	//	// Fills an array with count elements with the element passed by parameter
	//	template <typename T> void init(i32 count, const T& elem) {
	//		for (i32 i = 0; i < count; i++) { push_back(elem); }
	//	}

	//	// Adds the element at the end of the array
	//	void push_back(const T& Elem) { vec.push_back(Elem); }

	//	// Removes the last element of the array
	//	void pop_back() { dsverify(is_valid_index(size() - 1)); vec.pop_back(); }

	//}
	// Dynamic array
	// Now it uses the std::vector implementation for now
	template <typename T> struct darray {
		darray() = default;
		//darray(darray<T>&& other) { vec = std::move(other); }
		darray(const darray<T>& other) { vec = other.vec; }
		darray(std::initializer_list<T> l): vec(l) {}
		
		///// Insetion

		// Fills an array with count elements with the element passed by parameter
		void init(i32 count,const T& elem) {
			for (i32 i = 0; i < count; i++) { push_back(elem); }
		}

		// Adds the element at the end of the array
		void push_back(const T& Elem) { vec.push_back(Elem); }

		// Removes the last element of the array
		void pop_back() { dsverify(is_valid_index(size()-1)); vec.pop_back(); }



		// Set the number of array elements,
		// If the count number is greater that the current size, it adds copies of elem
		// If the count number is lower that the current size, it pops elements from the array.
		void resize(i32 count, const T& elem) { 
			if (size() == count) {
				return;
			} else {
				if (count > size()) {
					for (i32 i = 0; i < (count - size()); i++) push_back(elem);
				} else {
					for (i32 i = 0; i < (size() - count); i++) pop_back();
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
			i32 ele_idx_remove = -1;
			// TODO not optimized.. but it should work
			while (find(elem, &ele_idx_remove)) {
				remove_at(ele_idx_remove);
			}
		}

		void remove_single(const T& elem) {
			i32 ele_idx_remove = -1;
			if (find(elem, &ele_idx_remove)) {
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

		// Returns true only if the element was found. 
		// Sets the index parameter to the index of the first element found or -1 if no element is found
		bool find(const T& other, i32* index) const {
			dscheck(index != nullptr);
			for (i32 i = 0; i < size(); i++) {
				if (vec[i] == other) { *index = i;	return true; }
			}
			*index = -1; return false;
		}

		// Returns true only if the element was found. 
		// Sets the index parameter to the index of the last element found or -1 if no element is found
		bool find_last(const T& other, i32* last_index) const {
			dscheck(last_index != nullptr);
			bool found = false;
			*last_index = -1;
			for (i32 i = 0; i < size(); i++) {
				if (vec[i] == other) { *last_index = i; found = true; }
			}
			return found;
		}

		///// Sorting TODO

	private:
		i32 checked_size() const {
			dsverify(vec.size() < std::numeric_limits<i32>::max());
			return (i32)vec.size();
		}
		std::vector<T> vec;
	};
}

//namespace std {
//	template<> struct hash<ds::darray<ds::i32>> {
//		std::size_t operator()(const ds::darray<ds::i32> & Array) const {
//			size_t Seed = 0;
//			for (const auto& Elem : Array)
//				Seed ^= std::hash<int>()(Elem) + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
//			return Seed;
//		}
//	};
//}
