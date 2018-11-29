#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "envoy/common/pure.h"

#include "absl/strings/string_view.h"
#include "uv.h"

namespace Envoy {
namespace Event {

/**
 * Abstraction for a file watcher.
 */
class Watcher {
public:
  typedef std::function<void(const char* filename, int events, int status)> OnChangedCb;

  struct Events {
    static const int MovedTo = UV_RENAME;
    static const int Changed = UV_CHANGE;
  };

  virtual ~Watcher() {}

    /**
   * Add a file watch.
   * @param path supplies the path to watch.
   * @param cb supplies the callback to invoke when a change occurs.
   */
  virtual void addWatch(const std::string& path, OnChangedCb cb) PURE;
};

typedef std::unique_ptr<Watcher> WatcherPtr;

} // namespace Event
} // namespace Envoy
