#pragma once

// NOLINT(namespace-envoy)

// Macros that depend on the compiler
#if !defined(_MSC_VER)
#define PACKED_STRUCT(definition, ...) definition, ##__VA_ARGS__ __attribute__((packed))

#else

#include <malloc.h>

#define PACKED_STRUCT(definition, ...)                                                             \
  __pragma(pack(push, 1)) definition, ##__VA_ARGS__;                                               \
  __pragma(pack(pop))

#endif

// Macros that depend on the OS
#if !defined(WIN32)

#include <sys/socket.h>
#include <sys/uio.h>

typedef int SOCKET_FD;

typedef iovec IOVEC;
#define IOVEC_SET_BASE(iov, b) (iov).iov_base = (b)
#define IOVEC_SET_LEN(iov, l) (iov).iov_len = (l)

#define SOCKET_VALID(sock) ((sock) >= 0)
#define SOCKET_INVALID(sock) ((sock) == -1)
#define SOCKET_FAILURE(rc) ((rc) == -1)
#define SET_SOCKET_INVALID(sock) (sock) = -1

// arguments to shutdown
#define ENVOY_SHUT_RD SHUT_RD
#define ENVOY_SHUT_WR SHUT_WR
#define ENVOY_SHUT_RDWR SHUT_RDWR

#else

#include <stdint.h>
#include <winsock2.h>
// <winsock.h> includes <windows.h>, so undef some interfering symbols. DELETE
// shows up in the base.pb.h header generated from api/envoy/api/core/base.proto.
// Since it's a generated header, we can't just undef the symbol there.
// GetMessage show up in protobuf library code, so again we can't undef the
// symbol there.
#undef DELETE
#undef GetMessage

typedef SOCKET SOCKET_FD;

typedef _WSABUF IOVEC;
#define IOVEC_SET_BASE(iov, b) (iov).buf = static_cast<char*>((b))
#define IOVEC_SET_LEN(iov, l) (iov).len = (l)

#define SOCKET_VALID(sock) ((sock) != INVALID_SOCKET)
#define SOCKET_INVALID(sock) ((sock) == INVALID_SOCKET)
#define SOCKET_FAILURE(rc) ((rc) == SOCKET_ERROR)
#define SET_SOCKET_INVALID(sock) (sock) = INVALID_SOCKET

// arguments to shutdown
#define ENVOY_SHUT_RD SD_RECEIVE
#define ENVOY_SHUT_WR SD_SEND
#define ENVOY_SHUT_RDWR SD_BOTH

#if defined(_M_IX86)
typedef int32_t ssize_t;
#elif defined(_M_AMD64)
typedef int64_t ssize_t;
#else
#error add typedef for platform
#endif

typedef uint32_t mode_t;
#endif
