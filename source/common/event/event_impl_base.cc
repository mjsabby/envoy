#include "common/event/event_impl_base.h"

#include "uv.h"

namespace Envoy {
namespace Event {

ImplBase::~ImplBase() {
  // Derived classes are assumed to have already assigned the raw event in the constructor.
  uv_close(&raw_handle_.uv_handle_t, nullptr);
}

} // namespace Event
} // namespace Envoy
