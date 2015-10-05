/*
libTapir - Tapir vision toolkit interface
Copyright (c) 2010-2012 Machine Learning Lab, 
Thomas Lampe

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

#ifndef _KATCTR_SIMCAM_H_
#define _KATCTR_SIMCAM_H_

#include "tapir/carreracam.h"
#include "tapir/tapir.h"
#include "fli/flibase.h"

namespace Tapir
{

/** Provides a simulated camera mounted on the Katana's end-effector.
  * The camera's sensor is assumed to be located at the exact position of the TCP.
  * The current projection of the object is computed solely based on the position
  * of the virtual object provided at creation and the current pose of the robot.
  * Compatible with both real and simulated Katana, as well as both CarreraDetector
  * and TapirDetector interfaces. */
class KatanaSimCam : public CarreraDetector, TapirDetector
{
 public:
  KatanaSimCam (FLI::FLIBase* robot, double* obj, bool draw);
  ~KatanaSimCam () {;};
  void update () throw ();
  void set_object_position (double* obj);
 protected:
  int _i, _j, _k;
  const double *_pose;
  double _object_xyz[3], _rot1[3][3], _rot2[3][3], _rot3[3][3], _rot12[3][3], _roto[3][3], _ac[3], _d[3];
  FLI::FLIBase *_robot;
  bool _draw;
};

};

#endif
