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
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

#ifndef _observer_h_
#define _observer_h_

#include "global.h"

namespace CLS
{
/** @defgroup OBSERVER Observer Modules
  * The observer provides an intermediary between the plant and the controller.
  * It generates a system observation from the plant measurement that the controller can then base its action on. */

/** Base class for all observer modules in CLSquare. */
class Observer
{
public:
  /** init gets plant_state_dim, measurement_dim
   * \return observed_state_dim
   */

  virtual bool init(const int plant_state_dim, const int measurement_dim, const int action_dim,
                    int &observed_state_dim, const char *fname=0, const char *chapter=0) = 0;

  /** compute observed state out of measurement
   *  prev. information might be used to build up history
   *  to check, if history is valid, the cycle_ctr is also communicated
   * \return observed_state
   */

  virtual void get_observed_state(const double *prev_measurement, const double* prev_action,
                                  const double *current_measurement, const int cycle_ctr,
                                  double *observed_state) = 0;

  /** Notifies that an episode has been started. */
  virtual void notify_episode_starts() {
    return;
  }

  /** Notifies that an episode has been stopped. */
  virtual void notify_episode_stops() {
    return;
  }

  virtual void deinit() {
    ;
  }

  /** virtual destructor is necessary since methods declared virtual */
  virtual ~Observer() {}

protected:
  int __observed_state_dim;


};

};

#ifdef CLSQUARE
#include "registry.h"
#else
#define REGISTER(Observer, classname, desc)
#endif

#endif
