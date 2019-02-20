// enrico.gamberini@cern.ch

#ifndef EVENTBUILDER_H_
#define EVENTBUILDER_H_

#include <iostream>

#include "core/BaseClass.hpp"

class EventBuilder : public BaseClass
{
  public:
    EventBuilder();
    ~EventBuilder();

    void start();
    void stop();

    void runner();
};

#endif /* EVENTBUILDER_H_ */
