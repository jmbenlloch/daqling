// enrico.gamberini@cern.ch

#include "modules/EventBuilder.hpp"

#include <chrono>

#define __METHOD_NAME__ daq::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daq::utilities::className(__PRETTY_FUNCTION__)

using namespace std::chrono_literals;

extern "C" EventBuilder *create_object()
{
    return new EventBuilder;
}

extern "C" void destroy_object(EventBuilder *object)
{
    delete object;
}

EventBuilder::EventBuilder()
{
    INFO(__METHOD_NAME__ << " With config: " << m_config.dump() << " getState: " << this->getState() );
}

EventBuilder::~EventBuilder()
{
    INFO(__METHOD_NAME__);
}

void EventBuilder::start()
{
    INFO(__METHOD_NAME__ << " getState: " << getState() );

    m_runner_thread = std::make_unique<std::thread>(&EventBuilder::runner, this);
}

void EventBuilder::stop()
{
    INFO(__METHOD_NAME__ << " getState: " << this->getState() );
}

void EventBuilder::runner()
{
    auto &cm = daq::core::ConnectionManager::instance();
    
    INFO(__METHOD_NAME__ << " Running...");
    while (m_run)
    {
        std::string s1{cm.getStr(1)};
        std::string s2{cm.getStr(2)};
        if(s1 != "") {
            INFO("Received on channel 1 " << s1);
        }
        if(s2 != "") {
            INFO("Received on channel 2 " << s2);
        }
    }
    INFO(__METHOD_NAME__ << " Runner stopped");
}
