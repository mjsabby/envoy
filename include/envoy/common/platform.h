#pragma once
// NOLINT(namespace-envoy)

// This common "platform.h" header exists to simplify the most common references
// to non-ANSI C/C++ headers, required on Windows, Posix, Linux, BSD etc,
// and to provide substitute definitions when absolutely required.
//
// The goal is to eventually not require this file of envoy header declarations,
// but limit the use of these architecture-specific types and declarations
// to the corresponding .cc implementation files.

#ifdef _MSC_VER

#include <windows.h>
#include <winsock2.h>

// These must follow afterwards
#include <mswsock.h>
#include <ws2tcpip.h>

// <windows.h> defines some frequently used symbols, so we need to undef these interfering symbols.
#undef DELETE
#undef ERROR
#undef GetMessage
#undef interface
#undef TRUE

#include <io.h>
#include <stdint.h>

#define PACKED_STRUCT(definition, ...)                                                             \
  __pragma(pack(push, 1)) definition, ##__VA_ARGS__;                                               \
  __pragma(pack(pop))

using ssize_t = ptrdiff_t;

// This is needed so the OsSysCalls interface compiles on Windows,
// shmOpen takes mode_t as an argument.
using mode_t = uint32_t;

typedef unsigned int sa_family_t;

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

// Following Cygwin's porting example (may not be comprehensive)
#define SO_REUSEPORT SO_REUSEADDR

// Solve for rfc2292 (need to address rfc3542?)
#ifndef IPV6_RECVPKTINFO
#define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif

#else // POSIX

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/mman.h> // for mode_t
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h> // for iovec
#include <sys/un.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(__linux__)
#include <linux/netfilter_ipv4.h>
#endif

#define PACKED_STRUCT(definition, ...) definition, ##__VA_ARGS__ __attribute__((packed))

#ifndef IP6T_SO_ORIGINAL_DST
// From linux/netfilter_ipv6/ip6_tables.h
#define IP6T_SO_ORIGINAL_DST 80
#endif

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
