/*
clsquare - closed loop simulation system
Copyright (c) 2004, Neuroinformatics Group, Prof. Dr. Martin Riedmiller,
University of Osnabrueck
Copyright (c) 2011, Machine Learning Lab, Prof. Dr. Martin Riedmiller,
University of Freiburg

demoplant.cc: simple demo plant for demo purposes

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

#ifndef _EMOTIV_EPOC_CLIENT_H_
#define _EMOTIV_EPOC_CLIENT_H_

#define EMOTIV_SENSOR_DIM 14
#define EMOTIV_GENERAL_DIM 4
#define EMOTIV_DIM 32

#define FOR_EACH_EMOTIV_CHANNEL(_macro_) \
  _macro_(F3); \
  _macro_(FC6); \
  _macro_(P7); \
  _macro_(T8); \
  _macro_(F7); \
  _macro_(F8); \
  _macro_(T7); \
  _macro_(P8); \
  _macro_(AF4); \
  _macro_(F4); \
  _macro_(AF3); \
  _macro_(O2); \
  _macro_(O1); \
  _macro_(FC5);

#include "emokit/emokit.h"

namespace Emotiv
{

enum channel
{
  F3  = 0,
  FC6 = 1,
  P7  = 2,
  T8  = 3,
  F7  = 4,
  F8  = 5,
  T7  = 6,
  P8  = 7,
  AF4 = 8,
  F4  = 9,
  AF3 = 10,
  O2  = 11,
  O1  = 12,
  FC5 = 13
};

class Client
{
 public:
  inline emokit_frame get_data () {return _data;};
  virtual bool update () = 0;
  virtual inline bool init (const char* fname, const char* chapter) {return true;};
  virtual inline void deinit () {};

  template<class T, class U> static inline void get_channels (T& data, U* array)
  {
#define GET_CHANNEL(_channel_) array[Emotiv::_channel_] = data._channel_;
    FOR_EACH_EMOTIV_CHANNEL(GET_CHANNEL);
  };
  template<class T, class U> static inline void set_channels (T& data, U* array)
  {
#define SET_CHANNEL(_channel_) data._channel_ = array[Emotiv::_channel_];
    FOR_EACH_EMOTIV_CHANNEL(SET_CHANNEL);
  };
 protected:
  emokit_frame _data;
};

};

#endif
