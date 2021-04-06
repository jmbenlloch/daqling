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

#ifndef CONNECTION_MACROS_HPP
#define CONNECTION_MACROS_HPP
#include "Core/ConnectionLoader.hpp"
#include "Utils/Ers.hpp"
#include <boost/preprocessor/cat.hpp>

//#define REGISTER_PRODUCT(TYPE,NAME) void* blobs=DynamicFactory::AddConnection< TYPE > ( NAME );
#define REGISTER_SENDER(TYPE, NAME)                                                                \
  namespace {                                                                                      \
  struct BOOST_PP_CAT(SenderRegistrator, __LINE__) {                                               \
    BOOST_PP_CAT(SenderRegistrator, __LINE__)() {                                                  \
      ERS_DEBUG(0, "Hi from register sender, adding: " << (NAME));                                 \
      daqling::core::ConnectionLoader &instance = daqling::core::ConnectionLoader::instance();     \
      ERS_DEBUG(0, "Instance address:" << &instance);                                              \
      instance.addSender<TYPE>(NAME);                                                              \
    }                                                                                              \
  } BOOST_PP_CAT(registrator, __LINE__);                                                           \
  }
#define REGISTER_RECEIVER(TYPE, NAME)                                                              \
  namespace {                                                                                      \
  struct BOOST_PP_CAT(ReceiverRegistrator, __LINE__) {                                             \
    BOOST_PP_CAT(ReceiverRegistrator, __LINE__)() {                                                \
      daqling::core::ConnectionLoader::instance().addReceiver<TYPE>(NAME);                         \
    }                                                                                              \
  } BOOST_PP_CAT(registrator, __LINE__);                                                           \
  }
#define REGISTER_QUEUE(TYPE, NAME)                                                                 \
  namespace {                                                                                      \
  struct BOOST_PP_CAT(QueueRegistrator, __LINE__) {                                                \
    BOOST_PP_CAT(QueueRegistrator, __LINE__)() {                                                   \
      daqling::core::ConnectionLoader::instance().addQueue<TYPE>(NAME);                            \
    }                                                                                              \
  } BOOST_PP_CAT(registrator, __LINE__);                                                           \
  }
#endif