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

/// \cond
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>
/// \endcond

#include "Command.hpp"
#include "PluginManager.hpp"

using namespace daqling::core;
using namespace std::chrono_literals;

PluginManager::PluginManager() : m_create{}, m_delete{}, m_dp{}, m_loaded{false} {}

PluginManager::~PluginManager()
{
  if (m_handle) {
    m_delete(*m_dp);
    dlclose(*m_handle);
    m_loaded = false;
  }
}

bool PluginManager::load(std::string name)
{
  // Load the shared object
  std::string pluginName = "lib/libDaqlingModule" + name + ".so";
  m_handle = dlopen(pluginName.c_str(), RTLD_NOW);
  if (*m_handle == nullptr) {
    ERROR("Unable to dlopen module " << name << "; reason: " << dlerror());
    m_handle.reset();
    return false;
  }

  // Resolve functions for module creation/destruction
  try {
    m_create = resolve<CreateFunc>("daqling_module_create");
    m_delete = resolve<DeleteFunc>("daqling_module_delete");
  } catch (const std::runtime_error &) {
    return false;
  }

  m_dp = m_create(daqling::utilities::Logger::get_module_instance());
  m_loaded = true;
  return true;
}
