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

#ifndef _TAPIR_PLANT_H_
#define _TAPIR_PLANT_H_

#include "plant.h"
#include "tapir/tapir.h"
#include <vector>

/** Provides a two-way interface to an arbitrary number of Tapir instances.
  * Instances are defined by parameters of the form \b cam_i, with \e i
  * being a continuous index starting at zero. Each such parameter refers
  * to a configuration file in which the communication method for the
  * program instance is defined; ideally this is the same as the config
  * file that is used by Tapir itself.
  *
  * The plant state consists of a sequence of 4-tuples, which are in
  * turn composed of the last known object position, the object size (or
  * -1000 if the object is not visible), and the last known angle. One
  * tuple is present for each Tapir instance.
  *
  * The plant also accepts an action of the same dimensionality as the
  * number of program instances. Based on the action's value, a string
  * is sent to each instance, with the mapping defined by parameters of
  * the form \b command_i, with \e i being the value of the action.
  * Command definitions need to be continous and begin at 1, since an
  * action of 0 result in no message to be sent.
  *
  * @ingroup PLANT
  * @ingroup HARDWARE
  * @author Thomas Lampe
  **/
class TapirPlant : public CLS::Plant {
public:
  TapirPlant() {_check_visibility=NULL;};
  ~TapirPlant() {deinit();};
  bool get_next_plant_state (const double *current_state, const double *current_action, double *next_state);
  bool get_measurement (const double *state, double *measurement);
  bool check_initial_state (double *initial_plant_state);
  bool init (int& state_dim, int& measurement_dim, int& action_dim, double& delta_t, const char *fname=0, const char *chapter=0); 
  void deinit ();
  void notify_episode_starts ();

protected:
  bool get_camera_data (Tapir::TapirDetector *cam, double *dest, const double *last);
  std::vector<Tapir::TapirDetector*> _cam;
  std::vector<std::string> _commands;
  int _cams, _i, _current;
  bool *_check_visibility;
  std::vector<std::string> _reset_command;
  std::vector<bool> _reset;
};

#endif

