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

/**
 * @file ModuleManager.hpp
 * @brief Loads module (shared object) of type DAQProcess
 */

#ifndef DAQLING_CORE_ModuleManager_HPP
#define DAQLING_CORE_ModuleManager_HPP

/// \cond
#include <dlfcn.h>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
/// \endcond

#include "DAQProcess.hpp"
#include "ModuleLoader.hpp"
#include "Utils/ThreadTagger.hpp"
namespace daqling {
ERS_DECLARE_ISSUE(core, ModuleIssue, "", ERS_EMPTY)
ERS_DECLARE_ISSUE_BASE(core, InconsistentStates, core::ModuleIssue,
                       "Module states are inconsistent!", ERS_EMPTY, ERS_EMPTY)
ERS_DECLARE_ISSUE_BASE(core, InvalidModuleName, core::ModuleIssue,
                       "Couldn't find module with name: " << name, ERS_EMPTY, ((const char *)name))
namespace core {
struct module_info {
  std::unique_ptr<ModuleLoader> m_module_loader;
  std::string m_module_status = "booted";
  std::string m_module_type;
  std::string m_module_name;
};

// Could be simplified to one class with variadic template arguments.
class ModuleFunction {
  virtual bool operator()(const std::string &str, module_info &mod) const = 0;
};
class SimpleModuleFunction : public ModuleFunction {
public:
  SimpleModuleFunction(
      std::function<bool(const std::string &, module_info &, std::unordered_set<std::string>)> func,
      std::unordered_set<std::string> modargs) {
    m_modargs = std::move(modargs);
    m_func = std::move(func);
  }
  bool operator()(const std::string &str, module_info &mod) const override {
    return m_func(str, mod, m_modargs);
  }

private:
  std::unordered_set<std::string> m_modargs;
  std::function<bool(const std::string &, module_info &, std::unordered_set<std::string>)> m_func;
};
class StartModuleFunction : public ModuleFunction {
public:
  StartModuleFunction(std::function<bool(const std::string &, module_info &,
                                         std::unordered_set<std::string>, unsigned)>
                          func,
                      std::unordered_set<std::string> modargs, unsigned run_num) {
    m_modargs = std::move(modargs);
    m_func = std::move(func);
    m_run_num = run_num;
  }
  bool operator()(const std::string &str, module_info &mod) const override {
    return m_func(str, mod, m_modargs, m_run_num);
  }

private:
  std::unordered_set<std::string> m_modargs;
  std::function<bool(const std::string &, module_info &, std::unordered_set<std::string>, unsigned)>
      m_func;
  unsigned m_run_num;
};
class CustomModuleFunction : public ModuleFunction {
public:
  CustomModuleFunction(
      std::function<bool(const std::string &, module_info &, std::unordered_set<std::string>,
                         std::string, std::string)>
          func,
      std::unordered_set<std::string> modargs, std::string command, std::string arg) {
    m_modargs = std::move(modargs);
    m_func = std::move(func);
    m_command = std::move(command);
    m_arg = std::move(arg);
  }
  bool operator()(const std::string &str, module_info &mod) const override {
    return m_func(str, mod, m_modargs, m_command, m_arg);
  }

private:
  std::unordered_set<std::string> m_modargs;
  std::function<bool(const std::string &, module_info &, std::unordered_set<std::string>,
                     std::string, std::string)>
      m_func;
  std::string m_command;
  std::string m_arg;
};
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class ModuleManager : public daqling::utilities::Singleton<ModuleManager> {
private:
  std::map<std::string, module_info> m_modloaders;
  template <typename T> bool for_each_module(T & /*mod_func*/);
  // bool m_loaded;
  static daqling::utilities::ThreadTagger &m_tagger;
  static void setTag(std::string tag);

public:
  ModuleManager();
  ~ModuleManager();
  /**
   * Adds module_info structs for every module.
   * Sets module_status to "added", and provides modile_name.
   */
  void AddModules();
  /**
   * Tries to load a module of name `name`, with number 'no'.
   * Returns whether the operation succeeded.
   */

  bool load(const std::string &name, const std::string &type);

  // bool loadAll(const std::vector<std::string>& names);

  /**
   * Unloads the loaded module.
   * Returns whether the operation succeeded.
   */
  bool unload(std::unordered_set<std::string> modargs);

  /**
   * Configures the loaded module.
   *
   * @warning May only be called after a successful `load`.
   */
  void configure(std::unordered_set<std::string> modargs);
  /**
   * Unconfigures the loaded module.
   *
   * @warning May only be called after a successful `load`.
   */
  void unconfigure(std::unordered_set<std::string> modargs);

  /**
   * Starts the loaded module.
   *
   * @warning May only be called after a successful `load`.
   */
  void start(unsigned run_num, std::unordered_set<std::string> modargs);

  /**
   * Stops the loaded module.
   *
   * @warning May only be called after a successful `load`.
   */
  void stop(std::unordered_set<std::string> modargs);

  /**
   * Executes a custom module command `cmd` if registered.
   *
   * Returns whether specified command was executed.
   */
  bool command(const std::string &cmd, const std::string &arg,
               std::unordered_set<std::string> modargs);

  // std::string
  // getState(std::unordered_set<std::string> modargs = std::unordered_set<std::string>());
  std::string getState(const std::string &modarg);
  std::string getStatesAsString();
  std::vector<std::string> getIndividualStates();
  std::unordered_set<std::string>
  getModulesEligibleForCommand(std::unordered_set<std::string> modargs,
                               std::unordered_set<std::string> states);
  std::unordered_set<std::string>
  getModulesEligibleForCommand(std::unordered_set<std::string> modargs, const std::string &state);
  std::string getCommandTargetState(const std::string &, std::unordered_set<std::string> modarg);

  std::string getCommandTransitionState(const std::string &,
                                        std::unordered_set<std::string> modarg);

  std::string getReceiverType(const std::string &key) {
    return m_modloaders[key].m_module_loader->getReceiverType();
  }
  std::string getSenderType(const std::string &key) {
    return m_modloaders[key].m_module_loader->getSenderType();
  }

  // bool getLoaded() { return m_loaded; }
  std::unordered_set<std::string> CommandRegistered(const std::string & /*com*/,
                                                    std::unordered_set<std::string> modargs);
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_ModuleManager_HPP
