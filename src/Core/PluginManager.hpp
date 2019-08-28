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

/// \cond
#include <dlfcn.h>
#include <optional>
#include <string>
/// \endcond

#include "DAQProcess.hpp"

#include "Utils/Logging.hpp"

namespace daqling {
  namespace core {

    class PluginManager : public daqling::utilities::Singleton<PluginManager> {
  private:
      using CreateFunc = DAQProcess *(daqling::utilities::LoggerType);
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
      template <typename FuncSig> FuncSig *resolve(const char *symbol)
      {
        assert(*m_handle != nullptr);
        dlerror(); // discard any previous errors

        char *error;
        void *handle = dlsym(*m_handle, symbol);
        error = dlerror();
        if (error) {
          ERROR("Module resolution error: " << error);
          throw std::runtime_error("resolution error");
        }

        return reinterpret_cast<FuncSig *>(handle);
      }

  public:
      PluginManager();
      ~PluginManager();

      /**
       * Tries to load a module of name `name`.
       * Returns whether the operation succeeded.
       */
      bool load(std::string name);

      /**
       * Configures the loaded module.
       *
       * @warning May only be called after a successful `load`.
       */
      void configure() { m_dp.value()->configure(); };

      /**
       * Starts the loaded module.
       *
       * @warning May only be called after a successful `load`.
       */
      void start() { m_dp.value()->start(); };

      /**
       * Stops the loaded module.
       *
       * @warning May only be called after a successful `load`.
       */
      void stop() { m_dp.value()->stop(); };

      /**
       * Returns the state of the module.
       */
      std::string getState() { return m_dp.value()->getState(); }

      /**
       * Returns whether a module is loaded.
       */
      bool getLoaded() { return m_loaded; }
    };

  } // namespace core
} // namespace daqling

#endif // DAQLING_CORE_PLUGINMANAGER_HPP
