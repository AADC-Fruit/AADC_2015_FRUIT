/*
clsquare - closed loop simulation system
Copyright (c) 2010-2012 Machine Learning Lab, 
Prof. Dr. Martin Riedmiller, University of Freiburg

Author: Thomas Lampe

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
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE. 
*/

#include "emotivobserver.h"
#include "emotiv/pipe_client.h"
#include "emotiv/hid_client.h"
#include "emotiv/tcp_client.h"
#include "valueparser.h"
#include <cstring>

bool EmotivObserver::init (const int plant_state_dim, const int measurement_dim, const int action_dim, int &observed_state_dim, const char *fname, const char *chapter)
{
  CLS::ValueParser vp(fname, chapter==NULL ? "Observer" : chapter);
  _mdim = measurement_dim;
  observed_state_dim = _mdim + EMOTIV_DIM;

  // choose connection mode
  char con[255];
  sprintf(con, "%s", "pipe");
  vp.get("connection", con, 255);
  if (strcmp(con, "pipe") == 0) {
    IOUT("Connecting to EPOC via pipe. Please start the server program.");
    _emo = new Emotiv::PipeClient();
  } else if (strcmp(con, "hid") == 0) {
    IOUT("Connecting to EPOC directly.");
    _emo = new Emotiv::HIDClient();
 } else if (strcmp(con, "tcp") == 0) {
    IOUT("Connecting via TCP.");
    _emo = new Emotiv::TCPClient();
  } else {
    EOUT("Unknown connection type \"" << con << "\".");
    return false;
  }

  // connect to device
  if (!_emo->init(fname, chapter==NULL ? "Controller" : chapter)) {
    EOUT("Could not connect to device.");
    return false;
  }
  return true;
}

void EmotivObserver::get_observed_state (const double *prev_measurement, const double* prev_action, const double *current_measurement, const int cycle_ctr, double *observed_state)
{
  // keep original state
  for (_i=0; _i<_mdim; _i++)
    observed_state[_i] = current_measurement[_i];

  // add emotiv data
  _emo->update();
  emokit_frame frame = _emo->get_data();
  Emotiv::Client::get_channels<emokit_frame,double>(frame, &observed_state[_mdim]);
  Emotiv::Client::get_channels<emokit_contact_quality,double>(frame.cq, &observed_state[_mdim+EMOTIV_SENSOR_DIM]);
  observed_state[_mdim+2*EMOTIV_SENSOR_DIM]   = frame.counter;
  observed_state[_mdim+2*EMOTIV_SENSOR_DIM+1] = frame.battery;
  observed_state[_mdim+2*EMOTIV_SENSOR_DIM+2] = frame.gyroX;
  observed_state[_mdim+2*EMOTIV_SENSOR_DIM+3] = frame.gyroY;
}

void EmotivObserver::deinit ()
{
  if (_emo != NULL) {
    _emo->deinit();
    delete _emo;
    _emo = NULL;
  }
}

REGISTER(Observer, EmotivObserver, "Adds sensor data from an Emotiv EPOC headset.");

