#pragma once

#include "fixed_string.h"
#include "status.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace grpcxx {
template <fixed_string M, typename T, typename U> struct rpc {
	static constexpr std::string_view method{M};

	using method_type            = fixed_string_t<M>;
	using request_type           = T;
	using response_type          = U;
	using optional_response_type = std::optional<response_type>;

	struct result_type {
		class status           status;
		optional_response_type response;
	};

private:
	template <typename W> W map(std::string_view data) const {
		constexpr bool can_map = requires(W t) {
			{
				t.ParseFromArray(std::declval<const char *>(), std::declval<std::size_t>())
			} -> std::same_as<bool>;
		};
		static_assert(can_map, "No known method to deserialize data");

		W ret;

		if (!ret.ParseFromArray(data.data(), data.size())) {
			throw std::runtime_error("Failed to deserialize data");
		}

		return ret;
	}

	template <typename W> std::string map(const std::optional<W> &ret) const {
		constexpr bool can_map = requires(W t) {
			{ t.SerializeToString(std::declval<std::string *>()) } -> std::same_as<bool>;
		};
		static_assert(can_map, "No known method to serialize data");

		if (!ret) {
			return {};
		}

		std::string data;
		if (!ret->SerializeToString(&data)) {
			throw std::runtime_error("Failed to serialize data");
		}

		return data;
	}

public:
	request_type map_request(std::string_view data) const {
		return map<request_type>(std::move(data));
	}

	std::string map_request(const request_type &data) const { return map<request_type>(data); }

	response_type map_response(std::string_view data) const {
		return map<response_type>(std::move(data));
	}

	std::string map_response(const optional_response_type &ret) const {
		return map<response_type>(ret);
	}
};
} // namespace grpcxx
