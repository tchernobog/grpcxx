#pragma once

#include "../rpc.h"
#include "../service.h"

#include "coroutine.h"
#include "loop.h"
#include "scheduler.h"

#include <string_view>

namespace grpcxx::uv {

class client {
public:
	client(std::string_view ip, int port, detail::loop_t loop = {});
	client(const client &) = delete;
	client(client &&)      = delete;
	~client() noexcept     = default;

	auto operator=(const client &) -> client & = delete;
	auto operator=(client &&) -> client      & = delete;

protected:
	template <concepts::rpc_type Rpc>
	auto send(const typename Rpc::request_type &req) const noexcept -> typename Rpc::result_type {
		auto rpc  = Rpc{};
		auto data = rpc.map_request(req);

		// TODO: send and wait for answer from server.

		return typename Rpc::result_type{};
	}

private:
	detail::coroutine conn(uv_stream_t *stream);

	static void conn_cb(uv_stream_t *stream, int status);

	detail::loop_t    _loop;
	uv_tcp_t          _handle;
	detail::scheduler _scheduler;
};

} // namespace grpcxx::uv
