#pragma once

#include "envoy/event/timer.h"
#include "envoy/runtime/runtime.h"

#include "common/common/thread_impl.h"
#include "common/event/real_time_system.h"
#include "common/stats/thread_local_store.h"
#include "common/thread_local/thread_local_impl.h"

#include "exe/main_common_base.h"

#include "server/options_impl.h"
#include "server/server.h"
#include "server/test_hooks.h"

#ifdef ENVOY_HANDLE_SIGNALS
#include "exe/signal_action.h"
#include "exe/terminate_handler.h"
#endif

namespace Envoy {

// TODO(jmarantz): consider removing this class; I think it'd be more useful to
// go through MainCommonBase directly.
class MainCommon {
public:
  MainCommon(int argc, const char* const* argv);
  bool run() { return base_.run(); }

  // Makes an admin-console request by path, calling handler() when complete.
  // The caller can initiate this from any thread, but it posts the request
  // onto the main thread, so the handler is called asynchronously.
  //
  // This is designed to be called from downstream consoles, so they can access
  // the admin console information stream without opening up a network port.
  //
  // This should only be called while run() is active; ensuring this is the
  // responsibility of the caller.
  void adminRequest(absl::string_view path_and_query, absl::string_view method,
                    const MainCommonBase::AdminRequestFn& handler) {
    base_.adminRequest(path_and_query, method, handler);
  }

  static std::string hotRestartVersion(uint64_t max_num_stats, uint64_t max_stat_name_len,
                                       bool hot_restart_enabled);

private:
#ifdef ENVOY_HANDLE_SIGNALS
  Envoy::SignalAction handle_sigs;
  Envoy::TerminateHandler log_on_terminate;
#endif

  Envoy::OptionsImpl options_;
  Thread::ThreadFactoryImplPosix thread_factory_;
  MainCommonBase base_;
};

/**
 * This is the real main body that executes after site-specific
 * main() runs.
 *
 * @param options Options object initialized by site-specific code
 * @return int Return code that should be returned from the actual main()
 */
int main_common(OptionsImpl& options);

} // namespace Envoy
