#pragma once

#include "../rpc.h"
#include "../status.h"
#include "../stream.h"

#include "loop.h"
#include "uv.h"

#include <string_view>

namespace grpcxx::uv {

class client {
public:
	client(std::string_view ip, int port, detail::loop_t loop = {});
	client(uv_os_sock_t sockfd, detail::loop_t = {});

	client(const client &) = delete;
	client(client &&)      = delete;
	~client() noexcept;

	auto operator=(const client &) -> client & = delete;
	auto operator=(client &&) -> client      & = delete;

protected:
	template <concepts::rpc_type Rpc>
	auto send(const typename Rpc::request_type &req) noexcept -> typename Rpc::result_type {
		auto rpc  = Rpc{};
		auto data = rpc.map_request(req);
		// TODO:
		return {status::code_t::ok, rpc.map_response(send(std::move(data)))};
	}

	template <concepts::rpc_type Rpc>
	auto send_stream(const stream<typename Rpc::request_type> &req) noexcept ->
		typename Rpc::result_type {
		// TODO
	}

	template <concepts::rpc_type Rpc>
	auto send_stream(const typename Rpc::request_type &req) noexcept
		-> stream<typename Rpc::response_type> {
		// TODO
	}

	template <concepts::rpc_type Rpc>
	auto send_bidi(const stream<typename Rpc::request_type> &req) noexcept
		-> stream<typename Rpc::response_type> {
		// TODO
	}

	auto send(std::string data) -> std::string;

private:
	void connect();

	static void conn_cb(uv_connect_t *stream, int status);

	detail::loop_t   _loop;
	uv_tcp_t         _tcp_handle;
	sockaddr_storage _addr;
	uv_connect_t     _conn_handle;
	uv_stream_t     *_stream;
};

} // namespace grpcxx::uv
