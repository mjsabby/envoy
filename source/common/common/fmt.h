#pragma once

#include "absl/strings/string_view.h"
#include "fmt/format.h"
#include "fmt/ostream.h"

// fmt/format.h includes <windows.h>, so undef some interfering symbols
#if defined(WIN32)
#undef DELETE
#undef GetMessage
#endif

// NOLINT(namespace-envoy)

namespace fmt {

// Provide an implementation of formatter for fmt::format that allows absl::string_view to be
// formatted with the same format specifiers available to std::string.
// TODO(zuercher): Once absl::string_view is replaced with std::string_view, this can be removed
// as fmtlib handles std::string_view natively.
template <> struct formatter<absl::string_view> : formatter<string_view> {
  auto format(absl::string_view absl_string_view, fmt::format_context& ctx) -> decltype(ctx.out()) {
    string_view fmt_string_view(absl_string_view.data(), absl_string_view.size());
    return formatter<string_view>::format(fmt_string_view, ctx);
  }
};

} // namespace fmt
