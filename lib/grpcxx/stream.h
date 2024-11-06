#pragma once

#include "status.h"

#include <coroutine>
#include <exception>
#include <variant>

namespace grpcxx {

template <typename T>
// TODO: provide equality comparison, increment and
// inherit from public std::ranges::view_interface<stream<T>>
class stream {
public:
	struct promise_type;

	using value_type  = std::variant<void, T, status>;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type {
		value_type         _value;
		std::exception_ptr _exception;

		stream get_return_object() { return stream(handle_type::from_promise(*this)); }

		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }

		void unhandled_exception() { _exception = std::current_exception(); }

		template <std::convertible_to<T> From> std::suspend_always yield_value(From &&from) {
			_value = std::forward<From>(from); // caching the result in promise
			return {};
		}

		void return_void() {}
	};

	stream(handle_type h) : _h(h) {}
	virtual ~stream() noexcept { _h.destroy(); }

	explicit operator bool() {
		produce();
		return _h.done();
	}

	value_type operator()() {
		produce();
		// After the following move, the promise value
		// will be empty because the void variant will
		// be default constructed:
		return std::move(_h.promise()._value);
	}

private:
	handle_type _h;

	auto produce() -> void {
		if (!std::holds_alternative<status>(_h.promise()._value)) {
			_h();
			if (_h.promise().exception_) {
				std::rethrow_exception(_h.promise().exception_);
			}
		}
	}
};

} // namespace grpcxx
