#pragma once

#include <algorithm>
#include <string_view>

namespace grpcxx {
namespace detail {
template <class CharT, std::size_t N> struct basic_fixed_string {
	using value_type = CharT;

	consteval basic_fixed_string(const value_type (&str)[N + 1]) noexcept {
		std::copy_n(str, N + 1, value);
	}

	consteval operator std::basic_string_view<value_type>() const noexcept { return view(); }

	template <std::size_t M>
	consteval auto operator==(const basic_fixed_string<value_type, M> &rhs) const noexcept {
		return (N == M && view() == rhs.view());
	}

	[[nodiscard]] consteval const value_type *data() const noexcept { return &value[0]; }
	[[nodiscard]] consteval std::size_t       size() const noexcept { return N; }

	[[nodiscard]] consteval std::basic_string_view<value_type> view() const noexcept {
		return {&value[0], N};
	}

	value_type
		value[N + 1]; // extra space for null terminations when constructing from 'const ChatT *'
};
} // namespace detail

template <std::size_t N> struct fixed_string : detail::basic_fixed_string<char, N - 1> {
	consteval fixed_string(const char (&str)[N]) noexcept :
		detail::basic_fixed_string<char, N - 1>(str) {}
};

template <fixed_string T> struct fixed_string_t {};
} // namespace grpcxx
