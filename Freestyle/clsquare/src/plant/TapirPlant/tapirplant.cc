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

#include "tapirplant.h"
#include "valueparser.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>

bool TapirPlant::get_camera_data (Tapir::TapirDetector *cam, double *dest, const double *last)
{
  if (cam != NULL) {
    cam->update();
    if (cam->get_object().known) {
      dest[0] = cam->get_object().x;
      dest[1] = cam->get_object().y;
      dest[2] = cam->get_object().size;
      dest[3] = cam->get_object().angle;
      if (dest[2] < 1) dest[2] = 1.;
      return true;
    }
  }
  dest[0] = last[0];
  dest[1] = last[1];
  dest[2] = -1000.;
  dest[3] = last[3];
  return false;
}

bool TapirPlant::get_next_plant_state (const double *state, const double *action, double *next)
{
  for (_i=0; _i<_cams; _i++) {
    _current = int(action[_i]);
    if (_current > (int)_commands.size()) {
      WOUT(10, "No message for command (" << _current << ") specified; will ignore it.");
    } else if (_current != 0)
      _cam[_i]->notify(_commands[_current-1]);
  }

  for (_i=0; _i<_cams; _i++)
    get_camera_data(_cam[_i], &next[_i*4], &state[_i*4]);
  return true;
}

bool TapirPlant::get_measurement (const double *state, double *measurement)
{
  for (_i=0; _i<4*_cams; _i++)
    measurement[_i] = state[_i];
  return true;
}

bool TapirPlant::check_initial_state (double *state)
{
  double action[_cams], previous[_cams*4], size[_cams];
  for (_i=0; _i<_cams; _i++) {
    action[_i] = 0;
    size[_i]   = state[_i*4+2];
  }
  for (_i=0; _i<_cams*4; _i++)
    previous[_i] = 0;

  for (_i=0; _i<_cams; _i++)
    _cam[_i]->update();
  sleep(1);
  get_next_plant_state(previous, action, state);

  // note that we can't really check constraints here, since
  // a) we don't get the limits
  // b) we don't necessarily move to the starting pose before checking
  //
  // adding a special parameter would ruin the role of the init module;
  // so we only check for identical sign here
  for (_i=0; _i<_cams; _i++) {
    if (_check_visibility[_i] && state[_i*4+2] * size[_i] < 0)
      return false;
  }
  
  return true;
}

void TapirPlant::notify_episode_starts ()
{
  for (_i=0; _i<_cams; _i++)
    if (_reset[_i])
      _cam[_i]->notify(_reset_command[_i].c_str());
}


#define SSSET(xxx) { ss.str(""); ss << xxx; };
bool TapirPlant::init (int& state_dim, int& measurement_dim, int& action_dim, double& delta_t, const char *fname, const char *chapter)
{
  CLS::ValueParser vp(fname, chapter==0?"Plant":chapter);
  std::stringstream ss;

  // get camera configs
  for (_i=0; ; _i++) {
    char *config = new char[255];
    SSSET("cam_" << _i);
    if (vp.get(ss.str().c_str(), config, 255) < 1) break;
    if (strcmp(config, "false") == 0) break;
    IOUT("Loading configuration for camera " << _i << " from file " << config);
    _cam.push_back(new Tapir::TapirSharedMemoryDetector(config));
    delete config;

    char reset[255];
    SSSET("init_" << _i);
    if (vp.get(ss.str().c_str(), reset, 255) > 0) {
      _reset.push_back(true);
      _reset_command.push_back(std::string(reset));
    } else {
      _reset.push_back(false);
      _reset_command.push_back(std::string(""));
    }
  }
  _cams = _cam.size();
  if (_cams < 1) {
    EOUT("No cameras defined.");
    return false;
  }

  // read command dictionary
  for (_i=1; ; _i++) {
    char command[255];
    SSSET("command_" << _i);
    if (vp.get(ss.str().c_str(), command, 255) < 1) break;
    _commands.push_back(std::string(command));
  }

  _check_visibility = new bool[_cams];
  for (_i=0; _i<_cams; _i++)
    _check_visibility[_i] = false;
  vp.get("check_visibility", _check_visibility, _cams);

  // set dimensions
  state_dim = 4 * _cams;
  measurement_dim = 4 * _cams;
  action_dim = _cams;

  return true;
}

void TapirPlant::deinit ()
{
  for (int i=0; i<_cam.size(); i++)
    delete _cam[i];
  _cam.clear();

  if (_check_visibility != NULL)
    delete [] _check_visibility;
  _check_visibility = NULL;
}

REGISTER(Plant, TapirPlant, "Gets camera data and sends commands to Tapir.");

