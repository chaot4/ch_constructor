#pragma once

#include <type_traits>

namespace chc {
	/*
	 * convert enum value to underlying integer type
	 *
	 * unless given in enum declaration the underlying type is implementation
	 * defined
	 */
	template<typename Enum>
	typename std::underlying_type<Enum>::type from_enum(Enum val) {
		return static_cast<typename std::underlying_type<Enum>::type>(val);
	}

	/*
	 * convert integer value to given enum type; requires that the value
	 * is implicitly convertible to the underlying type
	 */
	template<typename Enum>
	Enum to_enum(typename std::underlying_type<Enum>::type val) {
		return static_cast<Enum>(val);
	}
}
