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

#include "MetricsSimulator.hpp"
#include "Core/Statistics.hpp"
#include <stdlib.h>

extern "C" MetricsSimulator *create_object() { return new MetricsSimulator; }

extern "C" void destroy_object(MetricsSimulator *object) { delete object; }

MetricsSimulator::MetricsSimulator() { INFO(""); }

MetricsSimulator::~MetricsSimulator() { INFO(""); }

void MetricsSimulator::start() {
  daqling::core::DAQProcess::start();
  m_metric1 = 0;
  m_metric2 = 0;
  m_metric3 = 0;
  m_metric4 = 0;
  m_metric5 = 0;
  m_metric6 = 0;
  m_metric7 = 0;
  m_metric8 = 0;
  m_metric9 = 0;
  m_metric10 = 0;
  m_statistics->registerVariable<std::atomic<int>, int>(&m_metric6, "RandomMetric1-int", daqling::core::metrics::LAST_VALUE, daqling::core::metrics::INT);
  m_statistics->registerVariable<std::atomic<float>, float>(&m_metric2, "RandomMetric2-float", daqling::core::metrics::LAST_VALUE, daqling::core::metrics::FLOAT);
  m_statistics->registerVariable<std::atomic<double>, double>(&m_metric3, "RandomMetric3-dobule", daqling::core::metrics::LAST_VALUE, daqling::core::metrics::DOUBLE);
  m_statistics->registerVariable<std::atomic<bool>, bool>(&m_metric4, "RandomMetric4-bool", daqling::core::metrics::LAST_VALUE, daqling::core::metrics::BOOL);
  m_statistics->registerVariable<std::atomic<size_t>, size_t>(&m_metric5, "RandomMetric5-size_t", daqling::core::metrics::LAST_VALUE, daqling::core::metrics::SIZE);
  m_statistics->registerVariable<std::atomic<int>, int>(&m_metric6, "RandomMetric6-int_average", daqling::core::metrics::AVERAGE, daqling::core::metrics::INT, 7);
  m_statistics->registerVariable<std::atomic<float>, float>(&m_metric2, "RandomMetric7-float_rate", daqling::core::metrics::RATE, daqling::core::metrics::FLOAT, 5);
  m_statistics->registerVariable<std::atomic<double>, double>(&m_metric3, "RandomMetric8-double_average", daqling::core::metrics::AVERAGE, daqling::core::metrics::DOUBLE, 5);
  m_statistics->registerVariable<std::atomic<bool>, bool>(&m_metric9, "RandomMetric9-bool", daqling::core::metrics::LAST_VALUE, daqling::core::metrics::BOOL);
  m_statistics->registerVariable<std::atomic<size_t>, size_t>(&m_metric5, "RandomMetric10-size_t_rate", daqling::core::metrics::RATE, daqling::core::metrics::SIZE, 5);
  INFO("");
}

void MetricsSimulator::stop() {
  daqling::core::DAQProcess::stop();
  INFO("");
}

void MetricsSimulator::runner() {
  INFO("Running...");
  srand (time(NULL));
  while (m_run) {
    m_metric1 += 4;
    m_metric6 = rand() % 10 + 1;
    m_metric2 = 100.5*(float)rand()/RAND_MAX;
    m_metric7 = (float)rand()/RAND_MAX;
    m_metric3 = 0.5*(double)rand()/RAND_MAX;
    m_metric8 = 10.*(double)rand()/RAND_MAX;
    if(m_metric4)
      m_metric4 = false;
    else
      m_metric4 = true;
    if(m_metric9)
      m_metric9 = false;
    else
      m_metric9 = true;
    
    m_metric5 += 50;
    m_metric10 = rand() % 100;
    std::this_thread::sleep_for( std::chrono::milliseconds(500) );
  }
  INFO("Runner stopped");
}
