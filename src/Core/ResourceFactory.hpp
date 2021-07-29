/**
 * Copyright (C) 2019-2021 CERN
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

#pragma once
#include "ConnectionLoader.hpp"
#include "DynamicLinkIssues.hpp"
#include "Utils/Resource.hpp"
#include "Utils/Singleton.hpp"
#include "nlohmann/json.hpp"
#include <dlfcn.h>
#include <zmq.hpp>
namespace daqling {
namespace core {
class ZMQ_Context : public daqling::utilities::Resource {
private:
  zmq::context_t m_context;

public:
  zmq::context_t *getContext() { return &m_context; }
};

class ResourceFactory : public daqling::utilities::Singleton<ResourceFactory> {
  using ProcessResourceMap = std::map<unsigned, std::shared_ptr<daqling::utilities::Resource>>;

private:
  ProcessResourceMap m_process_resources;
  std::unordered_map<std::string, std::function<std::unique_ptr<daqling::utilities::Resource>(
                                      const nlohmann::json &)>>
      m_loadable_resources;

  std::shared_ptr<daqling::utilities::Resource> createQueue(const nlohmann::json &json) {
    auto &cl = daqling::core::ConnectionLoader::instance();
    auto queueType = json.at("resource").at("type").get<std::string>();
    auto queue = cl.getQueue(queueType, json.at("resource"));
    return std::static_pointer_cast<daqling::utilities::Resource>(queue);
  }

public:
  void createResource(const nlohmann::json &json) {
    auto type = json.at("type").get<std::string>();
    auto id = json.at("id").get<unsigned>();
    // maybe check before overwriting index?
    if (type == "queue") {
      m_process_resources[id] = createQueue(json);
    } else if (type == "zmq_context") {
      m_process_resources[id] = std::make_shared<ZMQ_Context>();
    } else {
      if (m_loadable_resources.find(type) != m_loadable_resources.end()) {
        m_process_resources[id] = m_loadable_resources[type](json);
      } else {
        loadResource(type);
        if (m_loadable_resources.find(type) != m_loadable_resources.end()) {
          m_process_resources[id] = m_loadable_resources[type](json);
        } else {
          // throw invalid resource
        }
      }
    }
  }

  std::shared_ptr<daqling::utilities::Resource> getResource(unsigned id) {
    return m_process_resources[id];
  }

  template <class T> void addResource(const std::string &s) {
    m_loadable_resources[s] = [](const nlohmann::json &json) { return std::make_unique<T>(json); };
  }
  bool loadResource(const std::string &name) {
    std::string pluginName = "libDaqlingResource" + name + ".so";
    void *handle = dlopen(pluginName.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (handle == nullptr) {
      throw CannotOpenModule(ERS_HERE, name.c_str(), dlerror());
      return false;
    }
    return true;
  }
};

} // namespace core
} // namespace daqling