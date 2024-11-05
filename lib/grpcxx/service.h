#pragma once

#include "context.h"
#include "fixed_string.h"
#include "rpc.h"
#include "status.h"

#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>

namespace grpcxx {

template <fixed_string N, concepts::rpc_type... R> class service {
public:
	using response_t = std::pair<status, std::string>;

	using handler_t  = std::function<response_t(context &, std::string_view)>;
	using handlers_t = std::unordered_map<std::string_view, handler_t>;

	template <typename I> constexpr explicit service(I &impl) {
		std::apply(
			[&](auto &&...args) {
				auto helper = [&](const auto &rpc) {
					auto handler = [&impl,
									&rpc](context &ctx, std::string_view data) -> response_t {
						using type = std::remove_cvref_t<decltype(rpc)>;

						auto req    = rpc.map_request(data);
						auto result = std::invoke(&I::template call<type>, impl, ctx, req);

						return {result.status, rpc.map_response(result.response)};
					};

					_handlers.insert({rpc.method, handler});
				};

				(helper(args), ...);
			},
			std::tuple<R...>());
	}

	response_t call(context &ctx, std::string_view method, std::string_view data) {
		auto it = _handlers.find(method);
		if (it == _handlers.end()) {
			return {status::code_t::not_found, {}};
		}

		return it->second(ctx, data);
	}

	constexpr std::string_view name() const noexcept { return {N}; }

private:
	handlers_t _handlers;
};
} // namespace grpcxx
