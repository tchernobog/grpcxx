#include "client.h"

#include "uv/coroutine.h"

#include "uv.h"

#include <sys/socket.h>

#include <stdexcept>

namespace grpcxx::uv {

client::client(std::string_view ip, int port, detail::loop_t loop) :
	_loop(std::move(loop)), _stream(nullptr) {

	if (uv_ip4_addr(ip.data(), port, reinterpret_cast<sockaddr_in *>(&_addr)) != 0 &&
		uv_ip6_addr(ip.data(), port, reinterpret_cast<sockaddr_in6 *>(&_addr)) != 0) {
		throw std::runtime_error(std::string(ip) + " is not a valid IPv4 or IPv6 address");
	}

	_tcp_handle.data = this;
	uv_tcp_init(loop, &_tcp_handle);
}

client::~client() noexcept {
	uv_close(reinterpret_cast<uv_handle_t *>(&_conn_handle), nullptr);
	uv_close(reinterpret_cast<uv_handle_t *>(&_tcp_handle), nullptr);
}

auto client::send(std::string data) -> std::string {
	connect();
	// TODO: send and wait for answer from server.
}

auto client::connect() -> void {
	if (_stream != nullptr) {
		return; // Already connected
	}

	_conn_handle.data = this;
	// TODO: check if this leaks resources, as _conn_handle might already be initialized
	uv_tcp_connect(&_conn_handle, &_tcp_handle, reinterpret_cast<sockaddr *>(&_addr), &conn_cb);
	// TODO
}

auto client::conn_cb(uv_connect_t *conn, int status) -> void {
	if (status != 0) {
		throw std::runtime_error(
			std::string("unable to connect to gRPC server: ") + uv_strerror(status));
	}

	auto self           = static_cast<client *>(conn->data);
	self->_stream       = conn->handle;
	self->_stream->data = self;
}

} // namespace grpcxx::uv
