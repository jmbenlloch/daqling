// enrico.gamberini@cern.ch

#ifndef DAQ_CORE_PLUGINMANAGER_HPP
#define DAQ_CORE_PLUGINMANAGER_HPP

#include <dlfcn.h>
#include <string>
#include "modules/BoardReader.hpp"
#include "utilities/Logging.hpp"

namespace daq
{
namespace core
{

class PluginManager : public daq::utilities::Singleton<PluginManager>
{
  private:
    DAQProcess *(*m_create)(...);
    void (*m_destroy)(DAQProcess *);
    DAQProcess *m_dp;

  public:
    PluginManager();
    ~PluginManager();

    bool load(std::string name);
    void start() { m_dp->start(); };
    void stop() { m_dp->stop(); };
};

}
}

#endif
