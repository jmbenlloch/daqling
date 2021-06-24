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
  virtual void reconstruct(const void *data, size_t size) = 0;
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
template <class T> class DataFragment;
template <class T> class SharedDataType : public DataType {
  friend class DataFragment<T>;

private:
  std::shared_ptr<T> *hint_ptr;
  std::shared_ptr<T> data_ptr;
  bool m_shared = false;

public:
  ~SharedDataType() override = default;
  SharedDataType() : DataType(), data_ptr(std::make_shared<T>()){};
  SharedDataType(const void *data, const size_t size) { reconstruct(data, size); }
  /// Move constructor
  SharedDataType(SharedDataType<T> &&rhs) noexcept : DataType() {
    data_ptr = rhs.data_ptr;
    rhs.data_ptr = nullptr;
  }
  /// Move constructor DataFragment
  SharedDataType(DataFragment<T> &&rhs) noexcept {
    rhs.detach();
    data_ptr = std::shared_ptr<T>(rhs.data_ptr);
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
  /// Move assignment DataFragment
  SharedDataType &operator=(DataFragment<T> &&rhs) noexcept {
    if (this == &rhs) {
      return *this;
    }
    rhs.detach();
    data_ptr = std::shared_ptr<T>(rhs.data_ptr);
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
  /// Copy constructor DataFragment
  SharedDataType(const DataFragment<T> &rhs) noexcept {
    data_ptr = std::make_shared<T>(rhs.data(), rhs.size());
  }
  /// Copy assignment
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
  /// Copy assignment DataFragment
  SharedDataType<T> &operator=(const DataFragment<T> &rhs) {
    if (this == &rhs) {
      return *this;
    }
    if (rhs.data() != nullptr) {
      data_ptr = std::make_shared<T>(rhs.data(), rhs.size());
    } else {
      data_ptr = nullptr;
    }
    return *this;
  }
  void reconstruct(const void *data, const size_t size) override {
    data_ptr = std::make_shared<T>(data, size);
  }

  SharedDataType<T> &operator+=(SharedDataType<T> &rhs) {
    if (data_ptr != nullptr && rhs.data_ptr != nullptr) {
      *(data_ptr.get()) += *(rhs.data_ptr.get());
    } else if (rhs.data_ptr != nullptr) {
      data_ptr = std::make_shared<T>(rhs.data_ptr->data(), rhs.data_ptr->size());
    }
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
      return 0;
    }
    return data_ptr->size();
  }

  inline void *data() override {
    if (data_ptr == nullptr) {
      return nullptr;
    }
    return data_ptr->data();
  }

  inline const void *data() const override {
    if (data_ptr == nullptr) {
      return nullptr;
    }
    return data_ptr->data();
  }
  template <typename U = void *> U data() {
    static_assert(std::is_pointer<U>(), "Type parameter must be a pointer type");
    if (data_ptr == nullptr) {
      return static_cast<U>(nullptr);
    }
    return static_cast<U>(data_ptr->data());
  }
  template <typename U = void *> const U data() const {
    static_assert(std::is_pointer<U>(), "Type parameter must be a pointer type");
    if (data_ptr == nullptr) {
      return static_cast<U>(nullptr);
    }
    return static_cast<U>(data_ptr->data());
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
  friend class SharedDataType<T>;

private:
  T *data_ptr;
  bool m_detached{false};

public:
  ~DataFragment() override {
    if (!m_detached) {
      delete data_ptr;
    }
  };

  DataFragment() : data_ptr(nullptr){};
  DataFragment(const void *data, const size_t size) : data_ptr(nullptr) { reconstruct(data, size); }
  DataFragment(T *ptr) : data_ptr(ptr){};
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
  /// Copy assignment SharedDataType
  DataFragment<T> &operator=(const SharedDataType<T> &rhs) {
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
  void reconstruct(const void *data, const size_t size) override {
    delete data_ptr;
    data_ptr = new T(data, size);
  }

  DataFragment<T> &operator+=(DataFragment<T> &rhs) {
    if (data_ptr != nullptr && rhs.data_ptr != nullptr) {
      *data_ptr += *rhs.data_ptr;
    } else if (rhs.data_ptr != nullptr) {
      data_ptr = new T(rhs.data_ptr->data(), rhs.data_ptr->size());
    }
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
      return 0;
    }
    return data_ptr->size();
  }

  void *data() override {
    if (data_ptr == nullptr) {
      return data_ptr;
    }
    return data_ptr->data();
  }

  const void *data() const override {
    if (data_ptr == nullptr) {
      return data_ptr;
    }
    return data_ptr->data();
  }
  template <typename U = void *> U data() {
    static_assert(std::is_pointer<U>(), "Type parameter must be a pointer type");
    if (data_ptr == nullptr) {
      return static_cast<U>(nullptr);
    }
    return static_cast<U>(data_ptr->data());
  }
  template <typename U = void *> const U data() const {
    static_assert(std::is_pointer<U>(), "Type parameter must be a pointer type");
    if (data_ptr == nullptr) {
      return static_cast<U>(nullptr);
    }
    return static_cast<U>(data_ptr->data());
  }

  freeptr free() override {
    return [](void *, void *hint) { delete static_cast<T *>(hint); };
  }

  void *hint() override { return static_cast<void *>(data_ptr); }

  T *get() { return data_ptr; }

protected:
  void detach_data() override { m_detached = true; }
};

class DataTypeWrapper {
public:
  DataTypeWrapper(const DataTypeWrapper &) = delete;
  DataTypeWrapper(DataTypeWrapper &&rhs) noexcept
      : m_datatype(rhs.m_datatype), m_data(rhs.m_data), m_size(rhs.m_size),
        m_dealloc(rhs.m_dealloc) {
    rhs.m_dealloc = false;
  }
  DataTypeWrapper() = default;
  ~DataTypeWrapper() {
    if (m_dealloc) {
      delete m_datatype;
      // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
      free(m_data);
    }
  }
  DataTypeWrapper &operator=(const DataTypeWrapper &) = delete;
  DataTypeWrapper &operator=(DataTypeWrapper &&rhs) noexcept {
    if (&rhs == this) {
      return *this;
      // If rhs side type is already determined
    }
    if (rhs.m_datatype != nullptr) {
      std::swap(m_datatype, rhs.m_datatype);
      std::swap(m_dealloc, rhs.m_dealloc);
      // If this already knows its type
    } else if (this->m_datatype != nullptr && rhs.m_data != nullptr) {
      m_datatype->reconstruct(rhs.m_data, rhs.m_size);
      if (rhs.m_dealloc) {
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        free(rhs.m_data);
      }
      rhs.m_data = nullptr;
    } else {
      std::swap(m_data, rhs.m_data);
      std::swap(m_size, rhs.m_size);
      std::swap(m_dealloc, rhs.m_dealloc);
    }
    return *this;
  };
  template <class T> DataTypeWrapper(T &ptr) {
    static_assert(std::is_base_of<DataType, T>::value);
    m_datatype = new T(ptr);
    m_dealloc = true;
  }
  // NOLINTNEXTLINE(misc-forwarding-reference-overload)
  template <class T> DataTypeWrapper(T &&ptr) {
    static_assert(std::is_base_of<DataType, T>::value);
    m_datatype = new T(std::forward<T>(ptr));
    m_dealloc = true;
  }
  DataType *getDataTypePtr() { return m_datatype; }
  void reconstruct_or_store(void *data, size_t size) {
    if (m_datatype != nullptr) {
      m_datatype->reconstruct(data, size);
    } else {
      if (m_dealloc) {
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        free(m_data);
      }
      // NOLINTNEXTLINENEXTLINE(cppcoreguidelines-no-malloc)
      m_data = malloc(size);
      memcpy(m_data, data, size);
      m_size = size;
    }
    m_dealloc = true;
  }
  template <typename T> void transfer_into(T &ref) {
    static_assert(std::is_base_of<DataType, T>::value);
    if (m_datatype && m_dealloc) {
      ref = std::move(*(static_cast<T *>(m_datatype)));
      delete m_datatype;
      m_datatype = nullptr;
    } else if (m_data) {
      ref.reconstruct(m_data, m_size);
    } else {
      ERS_WARNING("Nothing to transfer!!!");
    }
  }

private:
  DataType *m_datatype{nullptr};
  void *m_data{nullptr};
  size_t m_size{0};
  bool m_dealloc = false;
};