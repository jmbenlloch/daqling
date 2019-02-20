// enrico.gamberini@cern.ch

#include "modules/EventBuilder.hpp"

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
    std::cout << "EventBuilder::EventBuilder" << std::endl;
}

EventBuilder::~EventBuilder()
{
    std::cout << "EventBuilder::~EventBuilder" << std::endl;
}

void EventBuilder::start()
{
    std::cout << "EventBuilder::start" << std::endl;
}

void EventBuilder::stop()
{
    std::cout << "EventBuilder::stop" << std::endl;
}

void EventBuilder::runner()
{
    
}