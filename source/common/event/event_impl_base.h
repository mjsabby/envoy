#pragma once

#include "uv.h"

namespace Envoy {
namespace Event {

/**
 * Base class for libevent event implementations. The event struct is embedded inside of this class
 * and derived classes are expected to assign it inside of the constructor.
 */
class ImplBase {
protected:
  ~ImplBase();

  uv_any_handle raw_handle_;
};

} // namespace Event
} // namespace Envoy
