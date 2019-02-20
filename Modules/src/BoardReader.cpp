// enrico.gamberini@cern.ch

#include "modules/BoardReader.hpp"

extern "C" BoardReader *create_object()
{
    return new BoardReader;
}

extern "C" void destroy_object(BoardReader *object)
{
    delete object;
}

BoardReader::BoardReader()
{
    m_run = false;
    std::cout << "BoardReader::BoardReader()" << std::endl;
}

BoardReader::~BoardReader()
{
    std::cout << "BoardReader::~BoardReader()" << std::endl;
}

void BoardReader::start()
{
    std::cout << "BoardReader::start" << std::endl;
}

void BoardReader::stop()
{
    std::cout << "BoardReader::stop" << std::endl;
}

void BoardReader::runner()
{
    while (m_run)
    {
        std::cout << "BoardReader::runner" << std::endl;
    }
}
