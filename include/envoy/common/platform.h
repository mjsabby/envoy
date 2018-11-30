#pragma once

// NOLINT(namespace-envoy)

// Macros that depend on the compiler
#if defined(_MSC_VER)
#include <malloc.h>

#define PACKED_STRUCT(definition, ...)                                                             \
  __pragma(pack(push, 1)) definition, ##__VA_ARGS__;                                               \
  __pragma(pack(pop))

#else
#define PACKED_STRUCT(definition, ...) definition, ##__VA_ARGS__ __attribute__((packed))

#endif
