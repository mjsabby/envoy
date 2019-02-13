// NOLINT(namespace-envoy)
#include <iostream>
#include <string>

#include "exe/platform_impl.h"

#include "test/tools/router_check/router.h"

int main(int argc, char* argv[]) {
  Envoy::Options options(argc, argv);

  // We need this to ensure WSAStartup is called on Windows
  Envoy::PlatformImpl platform_impl_;

  try {
    Envoy::RouterCheckTool checktool =
        options.isProto() ? Envoy::RouterCheckTool::create(options.configPath())
                          : Envoy::RouterCheckTool::create(options.unlabelledConfigPath());

    if (options.isDetailed()) {
      checktool.setShowDetails();
    }

    bool is_equal = options.isProto()
                        ? checktool.compareEntries(options.testPath())
                        : checktool.compareEntriesInJson(options.unlabelledTestPath());
    // Test fails if routes do not match what is expected
    if (!is_equal) {
      return EXIT_FAILURE;
    }
  } catch (const Envoy::EnvoyException& ex) {
    std::cerr << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
