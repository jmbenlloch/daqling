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
#include "Utils/Ers.hpp"
#include "Utils/Resource.hpp"
#include "Utils/Singleton.hpp"
#include "nlohmann/json.hpp"
#include <zmq.hpp>
namespace daqling {
ERS_DECLARE_ISSUE(core, InvalidID, "Trying to retrieve resource with invalid ID - ID: " << id,
                  ((unsigned)id))
namespace core {
/*
 * ZMQ_Context
 * Description: zmq::context_t wrapped in a resource class.
 * Allows for sharing a zmq context, to utilize zmq inproc communication between modules in the same
 * component. Date: July 2021
 */
class ZMQ_Context : public daqling::utilities::Resource {
private:
  /**
   * @brief zmq context
   */
  zmq::context_t m_context;

public:
  /**
   * @brief get method for Context.
   * @return pointer to zmq context.
   */
  zmq::context_t *getContext() { return &m_context; }
};

/*
 * ResourceFactory
 * Description: Responsible for creating and storing resources for a daqling component.
 * Date: July 2021
 */
class ResourceFactory : public daqling::utilities::Singleton<ResourceFactory> {
  using ProcessResourceMap = std::map<unsigned, std::shared_ptr<daqling::utilities::Resource>>;

private:
  /**
   * @brief Map for storing created resources.
   * Key: unique id.
   * Value: shared pointer to resource.
   */
  ProcessResourceMap m_process_resources;
  /**
   * @brief Create a queue resource.
   * @param json json configuration to use when constructing queue.
   * @return shared_ptr to constructed queue resource.
   */
  std::shared_ptr<daqling::utilities::Resource> createQueue(const nlohmann::json &json) {
    auto &cl = daqling::core::ConnectionLoader::instance();
    auto queueType = json.at("resource").at("type").get<std::string>();
    auto queue = cl.getQueue(queueType, json.at("resource"));
    return std::static_pointer_cast<daqling::utilities::Resource>(queue);
  }

public:
  /**
   * @brief Create an arbitrary resource type and store it in the resource map.
   * @param json json object naming type and id of resource + configs to use when constructing
   * resource.
   */
  void createResource(const nlohmann::json &json) {
    auto type = json.at("type").get<std::string>();
    auto id = json.at("id").get<unsigned>();
    // maybe check before overwriting index?
    if (type == "queue") {
      m_process_resources[id] = createQueue(json);
    } else if (type == "zmq_context") {
      m_process_resources[id] = std::make_shared<ZMQ_Context>();
    } else {
      // throw issue
    }
  }
  /**
   * @brief Get a resource identified by a certain id.
   * @param id id identifying the resource to retreive.
   * @return shared pointer to resource with specified id.
   */
  std::shared_ptr<daqling::utilities::Resource> getResource(unsigned id) {
    if (m_process_resources.find(id) != m_process_resources.end()) {
      return m_process_resources[id];
    }
    throw InvalidID(ERS_HERE, id);
  }
};

} // namespace core
} // namespace daqling