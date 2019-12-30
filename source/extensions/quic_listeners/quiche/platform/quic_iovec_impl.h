#pragma once

// NOLINT(namespace-envoy)

// This file is part of the QUICHE platform implementation, and is not to be
// consumed or referenced directly by other Envoy code. It serves purely as a
// porting layer for QUICHE.

// TODO(danzh) Add Windows support for iovec.
// Only works in platforms supports POSIX for now.

#if defined(WIN32)
/* Structure for scatter/gather I/O. */
struct iovec {
  void* iov_base; /* Pointer to data. */
  size_t iov_len; /* Length of data. */
};
#else
#include <sys/uio.h>

#endif // defined(OS_WIN) || defined(OS_NACL)
