// NOLINT(namespace-envoy)
#include "envoy/thread/thread.h"

#include "test/test_common/environment.h"
#include "test/test_common/utility.h"
#include "test/test_runner.h"

#include "absl/debugging/symbolize.h"

#ifdef ENVOY_HANDLE_SIGNALS
#include "common/signal/signal_action.h"
#endif

// The main entry point (and the rest of this file) should have no logic in it,
// this allows overriding by site specific versions of main.cc.
int main(int argc, char** argv) {
#if defined(WIN32)
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
  wVersionRequested = MAKEWORD(2, 2);

  err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) {
    /* Tell the user that we could not find a usable */
    /* Winsock DLL. */
    printf("WSAStartup failed with error: %d\n", err);
    return 1;
  }
#endif

#ifndef __APPLE__
  absl::InitializeSymbolizer(argv[0]);
#endif
#ifdef ENVOY_HANDLE_SIGNALS
  // Enabled by default. Control with "bazel --define=signal_trace=disabled"
  Envoy::SignalAction handle_sigs;
#endif

  Envoy::TestEnvironment::setEnvVar("TEST_RUNDIR",
                                    (Envoy::TestEnvironment::getCheckedEnvVar("TEST_SRCDIR") + "/" +
                                     Envoy::TestEnvironment::getCheckedEnvVar("TEST_WORKSPACE")),
                                    1);

  // Select whether to test only for IPv4, IPv6, or both. The default is to
  // test for both. Options are {"v4only", "v6only", "all"}. Set
  // ENVOY_IP_TEST_VERSIONS to "v4only" if the system currently does not support IPv6 network
  // operations. Similarly set ENVOY_IP_TEST_VERSIONS to "v6only" if IPv4 has already been
  // phased out of network operations. Set to "all" (or don't set) if testing both
  // v4 and v6 addresses is desired. This feature is in progress and will be rolled out to all tests
  // in upcoming PRs.
  Envoy::TestEnvironment::setEnvVar("ENVOY_IP_TEST_VERSIONS", "all", 0);
  return Envoy::TestRunner::RunTests(argc, argv);
}
