/**
 * Copyright (C) 2021 CERN
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
#ifndef ERS_HPP
#define ERS_HPP

#include <ers/ers.h>

/** \def ERS_WARNING( message ) This macro sends the message to the ers::warning stream.
 */
#define ERS_WARNING(message)                                                                       \
  do {                                                                                             \
    ERS_REPORT_IMPL(ers::warning, ers::Message, message, ERS_EMPTY);                               \
  } while (0)

#endif