#pragma once

#include "defs.h"
#include "enum_helpers.h"

#include <iterator>

namespace chc {
	template<typename Elem, typename Enum, size_t Size>
	class enum_array {
	public:
		using value_type = Elem;
		using index_type = Enum;
		using reference = value_type&;
		using const_reference = value_type const&;
		using pointer = value_type*;
		using const_pointer = value_type const*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static constexpr size_t SIZE = Size;

		/* inner array - public for easy construction, similar to std::array */
		value_type _array[SIZE];

		constexpr size_t size() const { return Size; }
		constexpr size_t max_size() const { return Size; }

		reference at(index_type ndx) {
			size_t pos = size_t{from_enum(ndx)};
			if (!(pos < Size)) throw std::out_of_range("enum_array out of bounds access");
			return _array[pos];
		}

		constexpr const_reference at(index_type ndx) const {
			return (size_t{from_enum(ndx)} < Size) ? _array[size_t{from_enum(ndx)}] : throw std::out_of_range("enum_array out of bounds access");
		}

		reference operator[](index_type ndx) {
			size_t pos = size_t{from_enum(ndx)};
			debug_assert(pos < Size);
			return _array[pos];
		}

		constexpr const_reference operator[](index_type ndx) const {
			return _array[size_t{from_enum(ndx)}];
		}

		reference front() {
			debug_assert(0 < Size);
			return _array[0];
		}

		constexpr const_reference front() const {
			return _array[0];
		}

		reference back() {
			debug_assert(0 < Size);
			return _array[Size-1];
		}

		constexpr const_reference back() const {
			return _array[Size-1];
		}

		pointer data() {
			return _array;
		}

		const_pointer data() const {
			return _array;
		}

		iterator begin() { return _array; }
		const_iterator begin() const { return _array; }
		const_iterator cbegin() const { return _array; }

		iterator end() { return _array + Size; }
		const_iterator end() const { return _array + Size; }
		const_iterator cend() const { return _array + Size; }

		reverse_iterator rbegin() { return reverse_iterator(end()); }
		const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
		const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
		const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

		constexpr bool empty() const { return 0 == Size; }

		void fill(const value_type& value) {
			for (value_type& entry: *this) entry = value;
		}

		friend void swap(enum_array& a, enum_array& b) {
			using std::swap;
			for (size_t i = 0; i < Size; ++i) {
				swap(a._array[i], b._array[i]);
			}
		}
	};
}
