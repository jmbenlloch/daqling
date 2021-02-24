#ifndef DAQLING_MODULE_NAME
#error "This header should only be automatically included during the build with CMake"
#endif

#include "Utils/Ers.hpp"
#include <chrono>

#include DAQLING_MODULE_HEADER

namespace daqutils = daqling::utilities;
using namespace std::chrono_literals;

/*
 * Defines a hidden symbol that holds the module's logger instance.
 * All log entries made in the module will be done through this instance.
 * Must be hidden from the symbol table so that it is not aliased to daqling's own when the module
 * is dynamically loaded.
 */
namespace daqling::core {
extern "C" {
// forward-declare to satisfy -Werror=missing-declarations
DAQProcess *daqling_module_create();
void daqling_module_delete(DAQProcess *);
}

DAQProcess *daqling_module_create() {

  auto module = new DAQLING_MODULE_NAME();
  return static_cast<DAQProcess *>(module);
}

void daqling_module_delete(DAQProcess *module) {
  while (module->running()) {
    // DAQProcess::m_runner_thread has yet to join,
    // we cannot safely destruct resources in derived module that m_runner_thread may be using.
    std::this_thread::sleep_for(1ms); // TODO: use a cv instead
  }
  delete module;
}
} // namespace daqling::core
