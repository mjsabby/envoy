/**
 * Utility to convert v1 JSON configuration file to v2 bootstrap JSON (on stdout).
 *
 * Usage:
 *
 * v1_to_bootstrap <input v1 JSON path>
 */
#include <cstdlib>

#include "envoy/config/bootstrap/v2/bootstrap.pb.h"
#include "envoy/config/bootstrap/v2/bootstrap.pb.validate.h"

#include "common/api/api_impl.h"
#include "common/config/bootstrap_json.h"
#include "common/json/json_loader.h"
#include "common/protobuf/utility.h"
#include "common/stats/isolated_store_impl.h"
#include "common/stats/stats_options_impl.h"

#include "exe/platform_impl.h"

// NOLINT(namespace-envoy)
int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input v1 JSON path>" << std::endl;
    return EXIT_FAILURE;
  }

  Envoy::PlatformImpl platform_impl_;
  Envoy::Stats::IsolatedStoreImpl stats_store;
  Envoy::Api::Impl api(std::chrono::milliseconds(1000), platform_impl_.threadFactory(), stats_store,
                       platform_impl_.rawFileSystem());

  envoy::config::bootstrap::v2::Bootstrap bootstrap;
  auto config_json = Envoy::Json::Factory::loadFromFile(argv[1], api);
  Envoy::Stats::StatsOptionsImpl stats_options;
  Envoy::Config::BootstrapJson::translateBootstrap(*config_json, bootstrap, stats_options);
  Envoy::MessageUtil::validate(bootstrap);
  std::cout << Envoy::MessageUtil::getJsonStringFromMessage(bootstrap, true);

  return EXIT_SUCCESS;
}
