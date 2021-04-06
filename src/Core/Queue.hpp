#pragma once
#include "Utils/Binary.hpp"
namespace daqling {
namespace core {
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Queue {
public:
  // Virtual destructor
  virtual ~Queue() = default;

  virtual bool read(daqling::utilities::Binary &) = 0;
  virtual bool write(const daqling::utilities::Binary &) = 0;
  virtual bool sleep_read(daqling::utilities::Binary & /*bin*/);
  virtual bool sleep_write(const daqling::utilities::Binary & /*bin*/);
  virtual uint sizeGuess() = 0;
  virtual uint capacity() = 0;
  virtual void set_sleep_duration(uint ms);

protected:
  uint m_sleep_duration{};
};
} // namespace core
} // namespace daqling
