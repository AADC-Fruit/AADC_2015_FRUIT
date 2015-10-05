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

#include "emotivcontrol.h"
#include "emotiv/pipe_client.h"
#include "emotiv/hid_client.h"
#include "valueparser.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <sstream>

bool EmotivController::get_action (const double *state, double *action)
{
  _emo->update();
  emokit_frame data = _emo->get_data();
  _emo->get_channels<emokit_frame,double>(data, &action[0]);
  //_emo->get_channels<emokit_contact_quality,int>(data.cq, &_contact_quality[0]);

  // pad action
  for (_i=_edim; _i<_adim; _i++)
    action[_i] = 0.;

  return true;
}

bool EmotivController::init (const int observed_state_dim, const int action_dim, double delta_t, const char *fname, const char* chapter)
{
  CLS::ValueParser vp(fname, chapter==NULL ? "Controller" : chapter);

  _adim = action_dim;
  _edim = min(_adim, EMOTIV_SENSOR_DIM);

  char con[255];
  sprintf(con, "%s", "pipe");
  vp.get("connection", con, 255);
  if (strcmp(con, "pipe") == 0) {
    IOUT("Connecting to EPOC via pipe. Please start the server program."); 
    _emo = new Emotiv::PipeClient();
  } else if (strcmp(con, "hid") == 0) {
    IOUT("Connecting to EPOC directly.");
    _emo = new Emotiv::HIDClient();
  } else {
    EOUT("Unknown connection type \"" << con << "\".");
    return false;
  }
  if (!_emo->init(fname, chapter==NULL ? "Controller" : chapter)) return false;

  return true;
}

void EmotivController::deinit ()
{
  _emo->deinit();
}

REGISTER(Controller, EmotivController, "Allows controlling a plant through an Emotiv EPOC headset.")

