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

#ifndef _MIXCONTROL_OPERATION_H_
#define _MIXCONTROL_OPERATION_H_

#include "controller.h"
#include "MixControl/slave.h"
#include "utils/setdef.h"
#include <vector>

using namespace CLS;
using namespace CLS::Util;

/** Provides the basic interface for special MixControl Operations.
  * All operations share a few common parameters:
  *
  * Parameters of the form <b> controller_x = <int> </b>, with \e x being a continuous index starting at 0, describe sub-controllers
  * that the opertation is carried out with. The parameter is a single number, which refers to the respective sub-controller
  * specified by MixControl. For instance, <em> controller_0 = 2 </em> would cause the controller described by \e 
  * MixControl::controller_2 to be used as the first element of the operation. The number of subcontrollers to be thus specified
  * depends on the specific operation to be used.
  *
  * Optional parameters of the form <b> actions_x = <int>+ </b> may be used to re-order the action vector of a sub-controller. For
  * instance, <em> action_0 = 0 0 0 </em> and <em> action_1 = 2 4 5 </em> would cause action 0 of the primary controller to be
  * combined with actions 3, 4 and 5 of the secondary one. The length of the assignment is expected to be equal to the plant's
  * action dimensionality, or the the parameter \e MixControl::action_dim_x corresponding to the chosen controller.
  *
  * Note that operations are only usable when created from within MixControl, and cannot be used as an application's main
  * controller. Attempting to do so may result in undefined behaviour. **/
class MixControlOperation : public Controller {
  public:
    /** Second initialization method that is called once all controllers have been initialized.
      * This method should call MixControlOperation::check_params to maintain a reference to
      * the other modules and chech constraints. */
    virtual bool init (const std::vector<MixControlSlave> *slaves, const char* fname, const char* chapter, const int id) = 0;
    bool init (const int observed_state_dim, const int action_dim, double deltat, const char* fname=0, const char* chapter=0);
  protected:
    bool check_params (const std::vector<MixControlSlave> *slaves, const int id);
    int _i, _adim, _sdim, _slavenum, *_cons, **_acts;
    const std::vector<MixControlSlave> *_slaves;
    bool _init;
};

#endif
