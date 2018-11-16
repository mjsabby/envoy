#include "exe/main_common.h"

#include "absl/debugging/symbolize.h"

#if defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
// <winsock.h> includes <windows.h>, so undef some interfering symbols
#undef DELETE
#undef GetMessage
#endif

// NOLINT(namespace-envoy)

/**
 * Basic Site-Specific main()
 *
 * This should be used to do setup tasks specific to a particular site's
 * deployment such as initializing signal handling. It calls main_common
 * after setting up command line options.
 */
int main(int argc, char** argv) {
#ifndef __APPLE__
  // absl::Symbolize mostly works without this, but this improves corner case
  // handling, such as running in a chroot jail.
  absl::InitializeSymbolizer(argv[0]);
#endif
#if defined(WIN32)
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

  std::unique_ptr<Envoy::MainCommon> main_common;

  // Initialize the server's main context under a try/catch loop and simply return EXIT_FAILURE
  // as needed. Whatever code in the initialization path that fails is expected to log an error
  // message so the user can diagnose.
  try {
    main_common = std::make_unique<Envoy::MainCommon>(argc, argv);
  } catch (const Envoy::NoServingException& e) {
    return EXIT_SUCCESS;
  } catch (const Envoy::MalformedArgvException& e) {
    return EXIT_FAILURE;
  } catch (const Envoy::EnvoyException& e) {
    return EXIT_FAILURE;
  }

  // Run the server listener loop outside try/catch blocks, so that unexpected exceptions
  // show up as a core-dumps for easier diagnostis.
  return main_common->run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
