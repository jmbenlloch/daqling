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
 * @brief Loads module (shared object) of type DAQProcess
 */

#ifndef DAQLING_CORE_PLUGINMANAGER_HPP
#define DAQLING_CORE_PLUGINMANAGER_HPP

#include <dlfcn.h>
#include <optional>
#include <string>

#include "DAQProcess.hpp"

#include "Utils/Ers.hpp"

namespace daqling {

#include <ers/Issue.h>

ERS_DECLARE_ISSUE(core, DynamicLinkIssue, "Dynamic library name: " << dlName,
                  ((const char *)dlName))

ERS_DECLARE_ISSUE_BASE(core, CannotResolveModule, core::DynamicLinkIssue,
                       "Failed to resolve module - Reason: " << reason << " - ",
                       ((const char *)dlName), ((const char *)reason))

ERS_DECLARE_ISSUE_BASE(core, CannotOpenModule, core::DynamicLinkIssue,
                       "Failed to dlopen module - Reason: " << reason << " - ",
                       ((const char *)dlName), ((const char *)reason))
ERS_DECLARE_ISSUE(core, MissingCreateOrDelete, "Failed to resolve create and/or delete", ERS_EMPTY)
namespace core {

class PluginManager : public daqling::utilities::Singleton<PluginManager> {
private:
  using CreateFunc = DAQProcess *(); //(daqling::utilities::LoggerType);
  using DeleteFunc = void(DAQProcess *);

  CreateFunc *m_create;
  DeleteFunc *m_delete;

  std::optional<DAQProcess *> m_dp;
  std::optional<void *> m_handle;
  bool m_loaded;

  /**
   * Resolves a symbol from the loaded shared module.
   * Throws std::runtime_error if the symbol cannot be resolved.
   */
  template <typename FuncSig> FuncSig *resolve(const char *symbol) {
    ERS_PRECONDITION(*m_handle != nullptr);
    dlerror(); // discard any previous errors

    char *error;
    void *handle = dlsym(*m_handle, symbol);
    error = dlerror();
    if (error) {
      throw CannotResolveModule(ERS_HERE, symbol, error);
    }

    return reinterpret_cast<FuncSig *>(handle);
  }

public:
  PluginManager();
  ~PluginManager();
  PluginManager(PluginManager const &) = delete;            // Copy construct
  PluginManager(PluginManager &&) = delete;                 // Move construct
  PluginManager &operator=(PluginManager const &) = delete; // Copy assign
  PluginManager &operator=(PluginManager &&) = delete;      // Move assign

  /**
   * Tries to load a module of name `name`.
   * Returns whether the operation succeeded.
   */
  bool load(const std::string &name);

  /**
   * Unloads the loaded module.
   * Returns whether the operation succeeded.
   */
  bool unload();

  /**
   * Configures the loaded module.
   *
   * @warning May only be called after a successful `load`.
   */
  void configure() {
    ERS_PRECONDITION(m_loaded);
    m_dp.value()->configure();
  };

  /**
   * Starts the loaded module.
   *
   * @warning May only be called after a successful `load`.
   */
  void start(unsigned run_num) {
    ERS_PRECONDITION(m_loaded);
    m_dp.value()->start(run_num);
  };

  /**
   * Stops the loaded module.
   *
   * @warning May only be called after a successful `load`.
   */
  void stop() {
    ERS_PRECONDITION(m_loaded);
    m_dp.value()->stop();
  };

  /**
   * Returns whether specified command was executed.
   */

  bool isCommandRegistered(const std::string &key) {
    return m_dp.value()->isCommandRegistered(key);
  }

  /**
   * Executes a custom module command `cmd`.
   */
  void command(const std::string &cmd, const std::string &arg) { m_dp.value()->command(cmd, arg); }

  std::string getCommandTransitionState(const std::string &key) {
    return m_dp.value()->getCommandTransitionState(key);
  }

  std::string getCommandTargetState(const std::string &key) {
    return m_dp.value()->getCommandTargetState(key);
  }

  /**
   * Returns whether a module is loaded.
   */
  bool getLoaded() { return m_loaded; }
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_PLUGINMANAGER_HPP
