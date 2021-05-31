#pragma once
#include "DataFormat.hpp"
#include "Utils/Ers.hpp"
#include <vector>

ERS_DECLARE_ISSUE(daqling, InternalAllocationIssue,
                  "Internal allocation issue, message from exception: " << message,
                  ((const char *)message))
ERS_DECLARE_ISSUE(daqling, UninitializedData, "Internal data was nullptr.", ERS_EMPTY)
using freeptr = void (*)(void *, void *);
/**
 * Abstract base class for DAQling DataTypes.
 * Primary purpose is abstraction for sending datastructures with the DAQling connections.
 */
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class DataType {
public:
  /**
   * @brief Reconstructs inner data.
   * @param size size of the memory region to reconstruct.
   * @param data pointer to memory region.
   */
  virtual void reconstruct(size_t size, const void *data) = 0;
  /**
   * @brief Detaches ownership of inner data.
   */
  virtual DataType *detach() {
    detach_data();
    return this;
  }; // data() pointing to memory region with data, and size() delimiting the area. (destructor of
     // DataType should no longer destruct data.)
  virtual ~DataType() = default;
  /**
   * @brief returns ptr to free function, to be used with zmq.
   */
  virtual freeptr free() = 0;
  /**
   * @brief returns hint ptr, to pass to freeptr function
   */
  virtual void *hint() { return nullptr; };
  /**
   * @brief returns size of memory region.
   */
  virtual size_t size() const = 0;
  /**
   * @brief returns pointer to memory region
   */
  virtual void *data() = 0;
  /**
   * @brief returns const pointer to memory region
   */
  virtual const void *data() const = 0;

protected:
  /**
   * @brief subclass implementation of detaching data.
   */
  virtual void detach_data() = 0;
};

template <class T> class SharedDataType : public DataType {
private:
  std::shared_ptr<T> *hint_ptr;
  std::shared_ptr<T> data_ptr;
  bool m_shared = false;

public:
  ~SharedDataType() override = default;

  SharedDataType() : DataType(), data_ptr(std::make_shared<T>()){};
  SharedDataType(const void *data, const size_t size) { reconstruct(size, data); }
  /// Move constructor
  SharedDataType(SharedDataType<T> &&rhs) noexcept : DataType() {
    data_ptr = rhs.data_ptr;
    rhs.data_ptr = nullptr;
  }
  /// Move assignment
  SharedDataType &operator=(SharedDataType<T> &&rhs) noexcept {
    if (this == &rhs) {
      return *this;
    }
    data_ptr = rhs.data_ptr;
    rhs.data_ptr = nullptr;
    return *this;
  }
  /// Copy constructor
  SharedDataType(const SharedDataType<T> &rhs) : DataType() {
    if (rhs.m_shared) {
      data_ptr = rhs.data_ptr;
    } else if (rhs.data_ptr != nullptr) {
      data_ptr = std::make_shared<T>(rhs.data(), rhs.size());
    } else {
      data_ptr = nullptr;
    }
  }
  SharedDataType<T> &operator=(const SharedDataType<T> &rhs) {
    if (this == &rhs) {
      return *this;
    }
    if (rhs.m_shared) {
      data_ptr = rhs.data_ptr;
    } else if (rhs.data_ptr != nullptr) {
      data_ptr = std::make_shared<T>(rhs.data(), rhs.size());
    } else {
      data_ptr = nullptr;
    }
    return *this;
  }
  void reconstruct(const size_t size, const void *data) override {
    data_ptr = std::make_shared<T>(data, size);
  }

  SharedDataType<T> &operator+=(SharedDataType<T> &rhs) {
    *(data_ptr.get()) += *(rhs.data_ptr.get());
    return *this;
  }
  bool operator==(SharedDataType<T> &rhs) { return data_ptr == rhs.data_ptr; }
  bool operator!=(SharedDataType<T> &rhs) { return data_ptr != rhs.data_ptr; }
  bool operator<(SharedDataType<T> &rhs) { return data_ptr < rhs.data_ptr; }
  bool operator>(SharedDataType<T> &rhs) { return data_ptr > rhs.data_ptr; }
  bool operator<=(SharedDataType<T> &rhs) { return data_ptr <= rhs.data_ptr; }
  bool operator>=(SharedDataType<T> &rhs) { return data_ptr >= rhs.data_ptr; }

  void make_shared() { m_shared = true; }
  T *operator->() { return data_ptr.get(); }
  inline size_t size() const override {
    if (data_ptr == nullptr) {
      throw daqling::UninitializedData(ERS_HERE);
    }
    return data_ptr->size();
  }

  inline void *data() override {
    if (data_ptr == nullptr) {
      throw daqling::UninitializedData(ERS_HERE);
    }
    return data_ptr->data();
  }

  inline const void *data() const override {
    if (data_ptr == nullptr) {
      throw daqling::UninitializedData(ERS_HERE);
    }
    return data_ptr->data();
  }

  freeptr free() override {
    return [](void *, void *hint) {
      auto *ptr = static_cast<std::shared_ptr<T> *>(hint);
      delete ptr;
    };
  }

  void *hint() override { return static_cast<void *>(hint_ptr); }

  T *get() { return data_ptr.get(); }

protected:
  void detach_data() override { hint_ptr = new std::shared_ptr<T>(data_ptr); }
};

template <class T> class DataFragment : public DataType {
private:
  T *data_ptr;
  bool m_detached{false};

public:
  ~DataFragment() override {
    if (!m_detached) {
      delete data_ptr;
    }
  };

  DataFragment() : DataType(), data_ptr(nullptr){};
  DataFragment(const void *data, const size_t size) { this->reconstruct(size, data); }
  DataFragment(T *ptr) : DataType(), data_ptr(ptr){};
  /// Move constructor
  DataFragment(DataFragment<T> &&rhs) noexcept {
    m_detached = rhs.m_detached;
    data_ptr = rhs.data_ptr;
    rhs.data_ptr = nullptr;
  }
  /// Move assignment
  DataFragment &operator=(DataFragment<T> &&rhs) noexcept {
    if (this == &rhs) {
      return *this;
    }
    if (!m_detached) {
      delete data_ptr;
    }
    data_ptr = rhs.data_ptr;
    rhs.data_ptr = nullptr;
    m_detached = rhs.m_detached;
    return *this;
  }
  /// Copy constructor
  DataFragment(const DataFragment<T> &rhs) {
    m_detached = false;
    if (rhs.data_ptr != nullptr) {
      data_ptr = new T(rhs.data(), rhs.size());
    } else {
      data_ptr = nullptr;
    }
  }
  /// Copy assignment
  DataFragment<T> &operator=(const DataFragment<T> &rhs) {
    if (this == &rhs) {
      return *this;
    }
    if (!m_detached) {
      delete data_ptr;
    }
    m_detached = false;
    if (rhs.data_ptr != nullptr) {
      data_ptr = new T(rhs.data(), rhs.size());
    } else {
      data_ptr = nullptr;
    }
    return *this;
  }
  void reconstruct(const size_t size, const void *data) override {
    delete data_ptr;
    data_ptr = new T(data, size);
  }

  DataFragment<T> &operator+=(DataFragment<T> &rhs) {
    *data_ptr += *rhs.data_ptr;
    return *this;
  }
  bool operator==(DataFragment<T> &rhs) { return *data_ptr == *rhs.data_ptr; }
  bool operator!=(DataFragment<T> &rhs) { return *data_ptr != *rhs.data_ptr; }
  bool operator<(DataFragment<T> &rhs) { return *data_ptr < *rhs.data_ptr; }
  bool operator>(DataFragment<T> &rhs) { return *data_ptr > *rhs.data_ptr; }
  bool operator<=(DataFragment<T> &rhs) { return *data_ptr <= *rhs.data_ptr; }
  bool operator>=(DataFragment<T> &rhs) { return *data_ptr >= *rhs.data_ptr; }

  /**
   * @brief pointer semantics to access inner data.
   */
  T *operator->() { return data_ptr; }

  size_t size() const override {
    if (data_ptr == nullptr) {
      throw daqling::UninitializedData(ERS_HERE);
    }
    return data_ptr->size();
  }

  void *data() override {
    if (data_ptr == nullptr) {
      throw daqling::UninitializedData(ERS_HERE);
    }
    return data_ptr->data();
  }

  const void *data() const override {
    if (data_ptr == nullptr) {
      throw daqling::UninitializedData(ERS_HERE);
    }
    return data_ptr->data();
  }

  freeptr free() override {
    return [](void *, void *hint) { delete static_cast<T *>(hint); };
  }

  void *hint() override { return static_cast<void *>(data_ptr); }

  T *get() { return data_ptr; }

protected:
  void detach_data() override { m_detached = true; }
};

#define datatypeList                                                                               \
  (SharedDataType<daqling::utilities::Binary>)(DataFragment<data_t>)(                              \
      DataFragment<daqling::utilities::Binary>)
#define DATATYPE_TO_STRING(DATATYPE)                                                               \
  { BOOST_PP_STRINGIZE(DATATYPE) }
