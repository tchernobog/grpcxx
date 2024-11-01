#include "grpcxx/uv/server.h"
#include "gtest/gtest.h"

#include "ping_pong.grpcxx.pb.h"
#include "uv.h"

#include <fmt/core.h>
#include <sys/socket.h>

#include <cstring>
#include <stop_token>
#include <thread>

template <>
auto PingPong::ServiceImpl::call<PingPong::rpchello>(grpcxx::context &, const Ping &req)
	-> rpchello::result_type {
	Pong res;
	return {grpcxx::status::code_t::ok, res};
}

namespace /* anonymous */ {

struct fd_t {
	fd_t(int fd) : _fd(fd) {}
	fd_t(const fd_t &) = delete;
	fd_t(fd_t &&that) : _fd(that.release()) {}
	~fd_t() { close(_fd); }

	int release() {
		using std::swap;
		int ret = -1;
		swap(ret, _fd);
		return ret;
	}
	operator int() const { return _fd; }

	int _fd;
};

TEST(UvServer, run_with_address_and_stop) {
	grpcxx::uv::server    server{1};
	PingPong::ServiceImpl ping_pong;
	PingPong::Service     service{ping_pong};
	server.add(service);

	auto thr = std::jthread{
		[&server](std::stop_token stop_token) { server.run("::1", 0, std::move(stop_token)); }};

	// TODO: we should do an exchange here, as soon as grpcxx also provides
	// a client. So far this was tested manually via Postman.
}

TEST(UvServer, run_with_existing_fd_and_stop) {
	sockaddr_in addr;
	socklen_t   addr_len{sizeof(addr)};

	auto sockfd = fd_t{socket(AF_INET, SOCK_STREAM, 0)};
	ASSERT_GE(sockfd, 0);

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port        = htons(0); // 0 means any available port

	ASSERT_GE(0, bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)));
	auto port = ntohs(addr.sin_port);

	grpcxx::uv::server    server{1};
	PingPong::ServiceImpl ping_pong;
	PingPong::Service     service{ping_pong};
	server.add(service);

	auto fd  = sockfd.release(); // the server will take ownership, and it will close it.
	auto thr = std::jthread{
		[&server, fd](std::stop_token stop_token) { server.run(fd, std::move(stop_token)); }};

	// TODO: we should do an exchange here, as soon as grpcxx also provides
	// a client. So far this was tested manually via Postman.
}

TEST(UvServer, run_with_external_loop) {
	// Integration with other event loops, such as the one of libevent,
	// is possible. Here we just pass a pre-built uv_loop_t
	// which can be easily reused inside a different event loop.
	uv_loop_t uv_loop;
	uv_loop_init(&uv_loop);

	class server : public ::grpcxx::uv::server {
	public:
		server(uv_loop_t &loop) : ::grpcxx::uv::server{loop, 1} { prepare("127.0.0.1", 0); }
	};

	server                server{uv_loop};
	PingPong::ServiceImpl ping_pong;
	PingPong::Service     service{ping_pong};
	server.add(service);

	// TODO: we should do an exchange here, as soon as grpcxx also provides
	// a client. So far this was tested manually via Postman
	// (and UV_RUN_DEFAULT / UV_RUN_ONCE).

	// Here we manually run the loop
	uv_run(&uv_loop, UV_RUN_NOWAIT);

	// Standard cleanup procedure for libuv
	uv_walk(
		&uv_loop,
		[](uv_handle_t *handle, void *) {
			if (!uv_is_closing(handle)) {
				uv_close(handle, nullptr);
			}
		},
		nullptr);

	while (uv_loop_close(&uv_loop) == UV_EBUSY) {
		uv_run(&uv_loop, UV_RUN_ONCE);
	}
}

} // namespace
