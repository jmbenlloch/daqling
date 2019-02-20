// enrico.gamberini@cern.ch

#ifndef BASECLASS_HPP_
#define BASECLASS_HPP_

// #include <atomic>

class BaseClass
{
  public:
    virtual ~BaseClass(){};

    /* use virtual otherwise linker will try to perform static linkage */
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void runner() = 0;

  private:
    // std::atomic<bool> m_run;
};

#endif /* BASECLASS_HPP_ */
