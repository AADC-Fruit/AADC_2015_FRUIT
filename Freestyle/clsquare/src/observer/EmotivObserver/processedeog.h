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

#ifndef _PROCESSED_EOG_OBSERVER_H_
#define _PROCESSED_EOG_OBSERVER_H_

#include "observer.h"
#include <utility>
#include <cassert>
#include "utils/tcpsocket.h"

/** Gets pre-processed EOG events via TCP.
  *
  * @author Thomas Lampe
  * @ingroup OBSERVER */
class ProcessedEOGServer : public CLS::Observer {
  public:
    ProcessedEOGServer () {_server = NULL; _socket = NULL;};
    ~ProcessedEOGServer () {deinit();};
    bool init (const int plant_state_dim, const int measurement_dim, const int action_dim, int &observed_state_dim, const char *fname=0, const char *chapter=0);
    void get_observed_state (const double *prev_measurement, const double* prev_action, const double *current_measurement, const int cycle_ctr, double *observed_state);
    void deinit ();

  protected:
    std::pair<size_t, size_t> find_last_datapoint();

    int _i, _mdim, _length;
    CLS::Util::TCPserver *_server;
    CLS::Util::TCPsocket *_socket;
    char _buffer[MAX_STR_LEN];
    std::stringstream _input;
    size_t _pos;
    std::string _input_string;
};

#endif
