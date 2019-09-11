#pragma once

// NOLINT(namespace-envoy)
#ifdef _MSC_VER
#include <stdint.h>

#define PACKED_STRUCT(definition, ...)                                                             \
  __pragma(pack(push, 1)) definition, ##__VA_ARGS__;                                               \
  __pragma(pack(pop))

#ifdef _M_X64
using ssize_t = ptrdiff_t;
#else
#error Envoy is not supported on 32-bit Windows
#endif

// This is needed so the OsSysCalls interface compiles on Windows. Specifically, shmOpen takes
// mode_t as an argument. Note that the Windows implementation just PANICs, since shm_open is not
// a function that exists.
using mode_t = uint32_t;

#else
#define PACKED_STRUCT(definition, ...) definition, ##__VA_ARGS__ __attribute__((packed))

#endif

// TODO(YAEL) As the IoHandle work progresses, all the macros below should disappear
// Macros that depend on the OS
#ifdef WIN32
#include <winsock2.h>
// <winsock2.h> includes <windows.h>, so undef some interfering symbols.
#undef TRUE
#undef DELETE
#undef ERROR
#undef GetMessage

#define SOCKET_FD SOCKET

using IOVEC = _WSABUF;
#define IOVEC_SET_BASE(iov, b) (iov).buf = static_cast<char*>((b))
#define IOVEC_SET_LEN(iov, l) (iov).len = (l)

#define SOCKET_VALID(sock) ((sock) != INVALID_SOCKET)
#define SOCKET_INVALID(sock) ((sock) == INVALID_SOCKET)
#define SOCKET_FAILURE(rc) ((rc) == SOCKET_ERROR)
#define SET_SOCKET_INVALID(sock) (sock) = INVALID_SOCKET

// CMSG_FIRSTHDR and CMSG_NEXTHDR are already defined on windows
#define CMSG_DATA(msg) (WSA_CMSG_DATA(msg))

// arguments to shutdown
#define ENVOY_SHUT_RD SD_RECEIVE
#define ENVOY_SHUT_WR SD_SEND
#define ENVOY_SHUT_RDWR SD_BOTH
#else

#include <sys/socket.h>
#include <sys/uio.h>

#define SOCKET_FD int

using IOVEC = iovec;
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
#endif
