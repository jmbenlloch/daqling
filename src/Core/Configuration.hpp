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

#ifndef DAQLING_CORE_CONFIGURATION_HPP
#define DAQLING_CORE_CONFIGURATION_HPP

#include "nlohmann/json.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "Utils/Logging.hpp"
#include "Utils/Singleton.hpp"

namespace daqling {
namespace core {

class Configuration : public daqling::utilities::Singleton<Configuration> {
public:
  Configuration() {}
  ~Configuration() {}

  void clear() { m_config.clear(); }
  std::string dump() { return m_config.dump(); }
  void load(const std::string &jsonStr) { m_config = nlohmann::json::parse(jsonStr); }
  nlohmann::json &getConfig() { return m_config; }
  nlohmann::json &getSettings() { return m_config["settings"]; }
  nlohmann::json &getMetricsSettings() { return m_config["metrics_settings"]; }
  nlohmann::json &getConnections() { return m_config["connections"]; }
  unsigned getNumReceiverConnections() { return m_config["connections"]["receivers"].size(); }
  unsigned getNumSenderConnections() { return m_config["connections"]["senders"].size(); }

  std::string getName() { return m_config["name"]; };

  template <typename T> void set(const std::string &key, const T &value) { m_config[key] = value; }
  template <typename T> T get(const std::string &key) { return m_config[key]; }

private:
  nlohmann::json m_config;
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_CONFIGURATION_HPP
