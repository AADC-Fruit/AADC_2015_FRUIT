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

#include "MixControl/operation.h"
#include "valueparser.h"
#include "registry.h"
#include <stdio.h>
#include <cmath>
#include <cstring>

#define SSSET(xxx) { ss.str(""); ss << xxx; };

#define MAX_NUM_SLAVES 20

bool MixControlOperation::init (const int observed_state_dim, const int action_dim, double deltat, const char* fname, const char* chapter)
{
  _init = false;
  _adim = action_dim;
  _sdim = observed_state_dim;

  bool variable = _slavenum < 1;
  if (variable) _slavenum = MAX_NUM_SLAVES;

  _acts = new int*[_slavenum];
  _cons = new int[_slavenum];

  ValueParser vp(fname, chapter==NULL ? "Controller" : chapter);
  std::stringstream ss;

  for (_i=0; _i<_slavenum; _i++) {
    SSSET("controller_" << _i);
    vp.get(ss.str().c_str(), _cons[_i], -1);
    if (_cons[_i] < 0) {
      if (variable) {
        _slavenum = _i;
        return true;
      } else {
        EOUT("Slave controller " << _i << " not specified (" << _cons[_i] << ") in chapter [" << chapter << "]");
        return false;
      }
    }

    //_acts[_i] = new int[_adim];
    for (int k=0; k<_adim; k++)
      _acts[_i][k] = k;
    SSSET("actions_" << _i);
    vp.get(ss.str().c_str(), _acts[_i], _adim);
  }

  return true;
}

bool MixControlOperation::check_params (const std::vector<MixControlSlave> *slaves, const int id)
{
  for (_i=0; _i<_slavenum; _i++) {
    if (_cons[_i] >= id) {
      EOUT("Slave controller ID " << _cons[_i] << " surpasses operator ID " << id << ".");
      return false;
    }
  }

  _slaves = slaves;
  _init = true;

  return true;
}
