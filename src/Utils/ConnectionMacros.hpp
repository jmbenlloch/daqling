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
#include "Common/DataType.hpp"
#include "Core/ConnectionLoader.hpp"
#include "Utils/Ers.hpp"
#include <boost/preprocessor.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/seq.hpp>

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

#define QUEUE_REGISTRATION_MACRO(r, QUEUETYPE, DATATYPE)                                           \
  struct BOOST_PP_CAT(QueueRegistrator, BOOST_PP_CAT(__LINE__, r)) {                               \
    BOOST_PP_CAT(QueueRegistrator, BOOST_PP_CAT(__LINE__, r))() {                                  \
      /*NOLINTNEXTLINE*/                                                                           \
      daqling::core::ConnectionLoader::instance().addQueue<QUEUETYPE<DATATYPE>>(                   \
          BOOST_PP_STRINGIZE(BOOST_PP_CAT(QUEUETYPE, DATATYPE)));                                  \
    }                                                                                              \
  } BOOST_PP_CAT(registrator, BOOST_PP_CAT(__LINE__, r));

#define QUEUE_SENDER_RECEIVER_REGISTRATION_MACRO(r, data, DATATYPE)                                \
  struct BOOST_PP_CAT(QueueSenderReceiverRegistrator, BOOST_PP_CAT(__LINE__, r)) {                 \
    BOOST_PP_CAT(QueueSenderReceiverRegistrator, BOOST_PP_CAT(__LINE__, r))() {                    \
      daqling::core::ConnectionLoader::instance().addQueueSender<DATATYPE>(                        \
          BOOST_PP_STRINGIZE(DATATYPE));                                                           \
      daqling::core::ConnectionLoader::instance().addQueueReceiver<DATATYPE>(                      \
          BOOST_PP_STRINGIZE(DATATYPE));                                                           \
    }                                                                                              \
  } BOOST_PP_CAT(registrator, BOOST_PP_CAT(__LINE__, r));
#define REGISTER_QUEUE(TYPE)                                                                       \
  namespace {                                                                                      \
  BOOST_PP_SEQ_FOR_EACH(QUEUE_REGISTRATION_MACRO, TYPE, datatypeList)                              \
  }
#define REGISTER_QUEUE_SENDERS_AND_RECEIVERS()                                                     \
  namespace {                                                                                      \
  BOOST_PP_SEQ_FOR_EACH(QUEUE_SENDER_RECEIVER_REGISTRATION_MACRO, 0, datatypeList)                 \
  }
REGISTER_QUEUE_SENDERS_AND_RECEIVERS()
#endif