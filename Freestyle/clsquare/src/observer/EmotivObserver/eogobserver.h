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

#ifndef _EMOTIV_EOG_OBSERVER_H_
#define _EMOTIV_EOG_OBSERVER_H_

#include "observer.h"
#include "emotiv/emotiv_client.h"
#include <boost/thread.hpp>
#include <Eigen/Dense>

/** Gets EOG events from an Emotiv EPOC headset.
  *
  * @author Thomas Lampe
  * @ingroup OBSERVER */
class EOGObserver : public CLS::Observer {
  public:
    EOGObserver () {_thread = NULL; _emo = NULL;};
    ~EOGObserver () {deinit();};
    bool init (const int plant_state_dim, const int measurement_dim, const int action_dim, int &observed_state_dim, const char *fname=0, const char *chapter=0);
    void get_observed_state (const double *prev_measurement, const double* prev_action, const double *current_measurement, const int cycle_ctr, double *observed_state);
    void deinit ();

  protected:
    int _i, _mdim, _event;
    bool _quit;
    Emotiv::Client *_emo;
    boost::thread *_thread;
    enum {EOG_SWITCH=1, EOG_NEXT=2, EOG_PREV=3, EOG_LOCK=4};
    char _config[255];
    static void check_for_event_ica (Emotiv::Client* const emo, int* const event, const bool* const quit, const char* const fname);
    static void check_for_event_manual (Emotiv::Client* const emo, int* const event, const bool* const quit, const char* const fname);
    static bool init_matrix_from_file (const char* cfgname, const char* entry, Eigen::MatrixXd& matrix, const int rows, const int cols);
};

#endif
