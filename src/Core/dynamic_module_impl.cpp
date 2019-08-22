#ifndef DAQLING_MODULE_NAME
#error "This header should only be automatically included during the build with CMake"
#endif

#include <chrono>

#include "Utils/Logging.hpp"
#include DAQLING_MODULE_HEADER

namespace daqutils = daqling::utilities;
using namespace std::chrono_literals;

// TODO: explain
HEDLEY_PRIVATE
daqutils::LoggerType daqutils::Logger::m_logger;

namespace daqling::core {
  extern "C" {
  // declare to satisfy -Werror=missing-declarations
  DAQProcess *daqling_module_create(daqutils::LoggerType);
  void daqling_module_delete(DAQProcess *);
  }

  // and then define
  DAQProcess *daqling_module_create(daqutils::LoggerType logger)
  {
    assert(logger);
    // must be set first so that we get "[module]" during setup
    // TODO: improve docs
    daqutils::Logger::set_instance(logger);

    auto module = new DAQLING_MODULE_NAME();
    return static_cast<DAQProcess *>(module);
  }

  void daqling_module_delete(DAQProcess *module)
  {
    while (module->running()) {
      // DAQProcess::m_runner_thread has yet to join,
      // we cannot safely destruct resources in derived module that m_runner_thread may be using.
      std::this_thread::sleep_for(1ms); // TODO: use a cv instead
    }
    delete static_cast<DAQLING_MODULE_NAME *>(module);
  }
} // namespace daqling::core
