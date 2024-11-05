#pragma once

#ifndef GRPCXX_USE_ASIO
#include "uv/client.h"
#else
#warning There is no client implementation for ASIO yet
#endif

namespace grpcxx {
#ifndef GRPCXX_USE_ASIO
using client = uv::client;
#endif
} // namespace grpcxx
