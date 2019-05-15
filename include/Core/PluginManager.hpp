/**
 * Copyright (C) 2019 CERN
 * 
 * DAQling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * DAQling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with DAQling. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file PluginManager.hpp
 * @author Enrico.Gamberini@cern.ch
 * @brief Load shared objects of type DAQProcess
 * @date 2019-05-14
 */

#ifndef DAQLING_CORE_PLUGINMANAGER_HPP
#define DAQLING_CORE_PLUGINMANAGER_HPP

/// \cond
#include <dlfcn.h>
#include <string>
/// \endcond

#include "Core/DAQProcess.hpp"

#include "Utilities/Logging.hpp"

namespace daqling {
namespace core {

class PluginManager : public daqling::utilities::Singleton<PluginManager> {
 private:
  daqling::core::DAQProcess *(*m_create)(...);
  void (*m_destroy)(daqling::core::DAQProcess *);
  daqling::core::DAQProcess *m_dp;
  void *m_handle;
  bool m_loaded;

 public:
  PluginManager();
  ~PluginManager();

  bool load(std::string name);
  void start() { m_dp->start(); };
  void stop() { m_dp->stop(); };
  std::string getState() { return m_dp->getState(); }
  bool getLoaded() { return m_loaded; }
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_PLUGINMANAGER_HPP
