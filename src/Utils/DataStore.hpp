/**
 * Copyright (C) 2019 CERN
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

#ifndef DAQLING_UTILITIES_DATASTORE_HPP
#define DAQLING_UTILITIES_DATASTORE_HPP

/*
 * DataStore
 * Description:
 *   Generic data source with casters
 * Date: March 2019
 */

namespace daqling {

namespace utilities {

class DataStoreBase {
public:
  virtual ~DataStoreBase() {}
  template <class T> std::shared_ptr<T> &getAs();
  template <class T, class U> void setAs(const std::shared_ptr<U> &rhs);
};

template <typename T> class DataStore : public DataStoreBase {
public:
  DataStore(const std::shared_ptr<T> &ds) : m_ds(ds) {
    // std::cout << " woof: DataStore<T>(const T& ds): new instance of DataStore!" << std::endl;
  }
  std::shared_ptr<T> &getAs() { return m_ds; }
  void setAs(const std::shared_ptr<T> &rhs) {
    // std::cout << " woof: setAs()!" << std::endl;
    m_ds = rhs;
  }

private:
  std::shared_ptr<T> m_ds;
};

// Here's the trick: dynamic_cast rather than virtual
template <class T> std::shared_ptr<T> &DataStoreBase::getAs() {
  return dynamic_cast<DataStore<T> &>(*this).getAs();
}

template <class T, class U> void DataStoreBase::setAs(const std::shared_ptr<U> &rhs) {
  return dynamic_cast<DataStore<T> &>(*this).setAs(rhs);
}

} // namespace utilities

} // namespace daqling

#endif // DAQLING_UTILITIES_DATASTORE_HPP
