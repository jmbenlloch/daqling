/*
 *  StandardStream.h
 *  ers
 *
 *  Created by Serguei Kolos on 02.08.05.
 *  Copyright 2004 CERN. All rights reserved.
 *
 *  Modified by DAQling
 *  Copyright (C) 2021 CERN
 */
#pragma once
#include <ers/OutputStream.h>
#include <ers/StandardStreamOutput.h>

namespace ers {

/** This class streams an issue into standard C++ output stream.
 *
 * \author Serguei Kolos
 * \brief Single line, human readable format stream.
 */

template <class Device>
struct DaqlingStandardStream : public OutputStream, public StandardStreamOutput, public Device {
  using Device::device;

  DaqlingStandardStream() { ; }

  DaqlingStandardStream(const std::string &file_name) : Device(file_name) { ; }

  void write(const Issue &issue) override;

private:
  void report(std::ostream &out, const Issue &issue, bool cause = false);
};
} // namespace ers
