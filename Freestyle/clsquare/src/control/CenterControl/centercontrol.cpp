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

#include "centercontrol.h"
#include "valueparser.h"
#include <string.h>
#include <stdio.h>
#include <cmath>

CenterControl::CenterControl ()
{
  _ass = NULL;
  _target = _multi = _params = NULL;
}

bool CenterControl::init (const int observed_state_dim, const int action_dim, double delta_t, const char *fname, const char* chapter)
{
  // set dimensions
  _adim = action_dim;
  _sdim = observed_state_dim;
  CLS::ValueParser vp(fname, chapter==NULL ? "Controller" : chapter);

  vp.get("negative", _negative, true);
  vp.get("limit", _limit, true);
  vp.get("false_below", _fbl, -1.);

  // set centering targets
  _target = new double[_adim];
  for (_i=0; _i<_adim; _i++) _target[_i] = 0.;
  vp.get("targets", &_target[0], _adim);

  // set state-to-action assignment; use first (action_dim) states by default
  _ass = new int[_adim];
  for (_i=0; _i<_adim; _i++)
    _ass[_i] = _i < _sdim ? _i : 0;
  vp.get("assignment", &_ass[0], _adim);
  for (_i=0; _i<_adim; _i++)
    if (_ass[_i] < 0 || _ass[_i] >= _sdim) {
      EOUT("State assignment must be between 0 and " << (_sdim-1) << ", is " << _ass[_i]);
      return false;
  }

  // set multiplier
  _multi = new double[_adim];
  for (_i=0; _i<_adim; _i++)
    _multi[_i] = 1.;
  vp.get("multi", &_multi[0], _adim);

  // determine controller mode
  _limit = true;
  char paramstr[255];
  if (vp.get("mode",paramstr,255) >= 0) {
    if (strcmp(paramstr,"Proportional") == 0) {
      _mode = Proportional;
    } else if (strcmp(paramstr,"ThreePoint") == 0) {
      _mode = ThreePoint;
    } else if (strcmp(paramstr,"FivePoint") == 0) {
      _mode = FivePoint;
    } else if (strcmp(paramstr,"Static") == 0) {
      _mode = Static;
      _limit = false;
    } else {
      if (strcmp(paramstr,"BangBang") != 0)
        WOUT(10, "Unknown mode.");
      _mode = BangBang;
    }
  } else {
    WOUT(10, "No controller mode specified.");
    _mode = BangBang;
  }

  // set parameters
  int pcount = 0;
  switch (_mode) {

    case Proportional:
      _params = new double[_adim];
      _params[0] = 1.;
      pcount = vp.get("scaling", _params, _adim);
      for (int a=pcount; a<_adim; a++)
        _params[a] = _params[0];
      IOUT("Using proportional control.");
      break;

    case ThreePoint:
      _params = new double[_adim];
      _params[0] = 0.5;
      pcount = vp.get("margin", _params, _adim);
      for (int a=pcount; a<_adim; a++)
        _params[a] = _params[0];
      IOUT("Using three-point controller.");
      break;

    case FivePoint:
      _params = new double[2*_adim];
      _params[0] = .33;
      pcount = vp.get("inner", &_params[0], _adim);
      for (int a=pcount; a<_adim; a++)
        _params[a] = _params[0];
      _params[_adim] = .66;
      pcount = vp.get("outer", &_params[_adim], _adim);
      for (int a=pcount; a<_adim; a++)
        _params[a+_adim] = _params[_adim];
      IOUT("Using five-point controller.");
      break;

    case Static:
      _params = new double[_adim];
      _params[0] = 0.;
      pcount = vp.get("value", _params, _adim);
      for (int a=pcount; a<_adim; a++)
        _params[a] = _params[0];
      IOUT("Using static output with value.");
      break;

    case BangBang:
      IOUT("Using bang-bang control.");
    default:
      ;
  }

  vp.get("limit", _limit);

  // output
  std::stringstream ss;
  ss << "Parameters: ";
  ss << "state->action assignment [";
  for (_i=0; _i<_adim; _i++) {
    if (_i > 0) ss << " ";
    ss << _ass[_i];
  }
  ss << "], targets [";
  for (_i=0; _i<_adim; _i++) {
    if (_i > 0) ss << " ";
    ss << _target[_i];
  }
  ss << "]";
  IOUT(ss.str());

  return true;
}

bool CenterControl::get_action (const double *state, double *action)
{
  double s;
  for (_i=0; _i<_adim; _i++) {

    s = state[_ass[_i]] - _target[_i];
    switch (_mode) {
      case BangBang:
        action[_i] = s == 0. ? 0. : s < 0. ? -1. : 1.;
        break;
      case Proportional:
        action[_i] = s / _params[_i];
        break;
      case ThreePoint:
        action[_i] = fabs(s) < _params[_i] ? 0. : s < 0. ? -1. : 1.;
        break;
      case FivePoint:
        action[_i] = fabs(s) < _params[_i] ? 0. : fabs(s) < _params[_adim+_i] ? 0.5 : 1.;
        if (s < 0.) action[_i] *= -1;
        break;
      case Static:
        action[_i] = _params[_i];
        break;
      default:
        action[_i] = 0.;
    }

    action[_i] *= _multi[_i];
    if (_limit) {
      if (action[_i] < -1.) action[_i] = -1.;
      else if (action[_i] > 1.) action[_i] = 1.;
    }
    if (!_negative) {
      action[_i] += 1.;
      action[_i] /= 2.;
    }
    if (fabs(action[_i]) < _fbl) return false;
  }

  return true;
}

void CenterControl::deinit ()
{
  if (_ass != NULL)
    delete [] _ass;
  _ass = NULL;
  if (_target != NULL)
    delete [] _target;
  _target = NULL;
  if (_multi != NULL)
    delete [] _multi;
  _multi = NULL;
  if (_params != NULL)
    delete [] _params;
  _params = NULL;
}

CenterControl::~CenterControl ()
{
  deinit();
}

REGISTER_CONTROLLER(CenterControl, "Controller which computes an action based on the current state's distance from a defined set point.")
