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