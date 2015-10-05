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
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef _EMOTIV_EPOC_PIPE_CLIENT_H_
#define _EMOTIV_EPOC_PIPE_CLIENT_H_

#define EMOTIV_BUFFER_SIZE 14096

#include "emotiv/emotiv_client.h"
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <cassert>

namespace Emotiv
{

class PipeClient : public Client
{
 public:
  bool update ();
  inline bool init (const char* fname, const char* chapter);
  inline void deinit ();
 protected:
  std::pair<size_t, size_t> find_last_datapoint();

  int _pipe, _i, _length;
  char _buffer[EMOTIV_BUFFER_SIZE], *_pipename;
  std::stringstream _input;

  int _sensor_data[EMOTIV_SENSOR_DIM];
  int _contact_quality[EMOTIV_SENSOR_DIM];
  int _device_data[4];

  size_t _pos;
  std::string _input_string;
};

};

#endif
