/*
clsquare - closed loop simulation system
Copyright (c) 2004, Neuroinformatics Group, Prof. Dr. Martin Riedmiller,
University of Osnabrueck
Copyright (c) 2011, Machine Learning Lab, Prof. Dr. Martin Riedmiller,
University of Freiburg

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the <ORGANIZATION> nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "emotiv/hid_client.h"
#include "emotiv/global.h"
#include "emotiv/valueparser.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>


namespace Emotiv
{

bool HIDClient::update ()
{
  // nothing to do, all work is done by the thread
  return true;
}

bool HIDClient::init (const char* fname, const char* chapter)
{
  ValueParser vp(fname, chapter);
  _quit = false;
  _thread = new boost::thread(HIDClient::keep_reading, &_data, &_quit);
  return true;
}

void HIDClient::keep_reading (emokit_frame* const data, const bool* const quit)
{
  // connect to device
  struct emokit_device *device = emokit_create();
  int count = emokit_get_count(device, EMOKIT_VID, EMOKIT_PID);
  IOUT("Current EPOC devices connected: " << count);
  int res = emokit_open(device, EMOKIT_VID, EMOKIT_PID, count-1);
  if (res != 0) {
    EOUT("Could not connect to EPOC (error " << res << ")");
    return;
  }
  IOUT("Successfully connected to EPOC device.");

  // read data
  emokit_frame current;
  while (!*quit) {
    bool read_data = false;
    while (!read_data)
      read_data = emokit_read_data(device) > 0;
    current = emokit_get_next_frame(device);
#define COPY_CHANNELS(_channel_) \
    data->_channel_    = current._channel_; \
    data->cq._channel_ = current.cq._channel_;
    FOR_EACH_EMOTIV_CHANNEL(COPY_CHANNELS);
    data->gyroX = current.gyroX;
    data->gyroY = current.gyroY;
    data->counter = current.counter;
    data->battery = current.battery;
  }
}

void HIDClient::deinit ()
{
  _quit = true;
  IOUT("Waiting for EPOC data thread to finish.");
  _thread->join();
}

};
