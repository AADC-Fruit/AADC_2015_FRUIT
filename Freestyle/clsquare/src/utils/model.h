/*
clsquare - closed loop simulation system
Copyright (c) 2014, Machine Learning Lab, Prof. Dr. Martin Riedmiller,
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

#ifndef _MODEL_H_
#define _MODEL_H_

#include "global.h"
#include "plant.h"

namespace CLS
{
namespace Util
{

/** @defgroup MODEL System models
  * A sub-type of plants with additional functionality to learn a model.
  * @ingroup PLANT */

/** Base class for all models in CLSquare.
  * Note that a model should usually not set the state and action dimensionality
  * in its init(), but rather accept those received from the calling class,
  * unless a previously saved model is being loaded.
  * \note When using objects of this class, make sure to call init_main instead of init(),
  *       otherwise implementations that do not override get_measurement() may crash. */
class Model : public CLS::Plant
{
  public:
    virtual ~Model() {;};

    /** Adds a new observed transition to the model.
      * \param observed_state state in which the transition started
      * \param action action taken in the observed state
      * \param next_observed_state state in which the transition ended
      * \param reward reward costs for transiton
      * \param is_terminal_state information if transition ends in terminal state
      * \param terminal_reward information about terminal_reward (only valid, if terminal state) */
    virtual void notify_transition (const double* state, const double* action, const double* next_state) {;};

    /** Updates the model with collected transition data in case this is not
      * done automatically with every new transition. */
    virtual void update () {;};
};

/** Extended model class that provides variance or uncertainty measures.
  * It gives separate variances for all state dimensions. */
class VarianceModel : public CLS::Util::Model
{
  public:
    /** Gets the variance for each state dimension.
      * \param state current system state
      * \param action action taken in current state
      * \param variance array of same length as system state that will hold the variances */
    virtual void get_variance (const double* state, const double* action, double* variance) = 0;
};

/** Extended model class that provides derivatives.
  * It gives separate gradients for all state and action dimensions. */
class DifferentiableModel : public CLS::Util::Model
{
  public:
    /** Gets the derivative of the output for each state and action dimension.
      * \param state current system state
      * \param action action taken in current state
      * \param derivative array of combined length of system state + action that will hold the derivatives */
    virtual void get_derivative (const double* observed_state, const double* action, double* derivative) = 0;
};

};
};

#endif
