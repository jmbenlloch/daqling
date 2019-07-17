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

#include "Core/Command.hpp"
#include "Core/PluginManager.hpp"


using namespace daqling::core;
using namespace std::chrono_literals;

PluginManager::PluginManager() : m_create{}, m_destroy{}, m_dp{}, m_loaded{false} {}

PluginManager::~PluginManager() {
  if (m_handle != 0) {
    m_destroy(m_dp);
    m_loaded = false;
  }
}

bool PluginManager::load(std::string name) {
  std::string pluginName = "lib" + name + ".so";
  m_handle = dlopen(pluginName.c_str(), RTLD_LAZY);
  if (m_handle == 0) {
    ERROR("Plugin not loaded!");
    return false;
  }

  m_create = (DAQProcess * (*)(...)) dlsym(m_handle, "create_object");
  m_destroy = (void (*)(DAQProcess *))dlsym(m_handle, "destroy_object");

  m_dp = (DAQProcess *)m_create();
  m_loaded = true;
  return true;
}

bool PluginManager::configure() {
  m_dp->setupStatistics();
  return true;
}

