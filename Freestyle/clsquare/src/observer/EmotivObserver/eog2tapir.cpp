/*
clsquare - closed loop simulation system
Copyright (c) 2010-2013 Machine Learning Lab, 
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

#include "eog2tapir.h"
#include "valueparser.h"

bool EOG2Tapir::get_action(const double* observed_state, double* action)
{
  if (observed_state[0] <= -1.)
    action[0] = actions[2];
  else if (observed_state[0] >= 1.)
    action[0] = actions[1];
  else
    action[0] = actions[0];
  return true;
}

bool EOG2Tapir::init(const int odim, const int adim, double dt, const char* fname, const char* chapter)
{
  if (adim != 1 || odim != 1) {
    EOUT("Observation and action dimensions should be 1!");
    return false;
  }

  CLS::ValueParser vp(fname, chapter==0?"Controller":chapter);
  for (int i=0; i<3; i++) actions[i] = 0.;
  vp.get("actions", actions, 3);
  return true;
}

REGISTER(Controller, EOG2Tapir, "Utility controller to transform EOG directions to Tapir events.");
