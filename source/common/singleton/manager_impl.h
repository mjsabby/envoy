#pragma once

#include <unordered_map>

#include "envoy/api/api.h"
#include "envoy/singleton/manager.h"
#include "envoy/thread/thread.h"

namespace Envoy {
namespace Singleton {

/**
 * Implementation of the singleton manager that checks the registry for name validity. It is
 * assumed the singleton manager is only used on the main thread so it is not thread safe. Asserts
 * verify that.
 */
class ManagerImpl : public Manager {
public:
  ManagerImpl(Api::Api& api) : run_tid_(api.currentThreadId()) {}

  // Singleton::Manager
  InstanceSharedPtr get(const std::string& name, SingletonFactoryCb cb) override;

private:
  std::unordered_map<std::string, std::weak_ptr<Instance>> singletons_;
  Thread::ThreadIdPtr run_tid_;
};

} // namespace Singleton
} // namespace Envoy
