// enrico.gamberini@cern.ch

#ifndef BOARDREADER_HPP_
#define BOARDREADER_HPP_

#include <iostream>
#include <atomic>
#include "BaseClass.hpp"

class BoardReader : public BaseClass
{
    std::atomic<bool> m_run;

  public:
    BoardReader();
    ~BoardReader();
    void start();
    void stop();

    void runner();
};

#endif /* BOARDREADER_HPP_ */
