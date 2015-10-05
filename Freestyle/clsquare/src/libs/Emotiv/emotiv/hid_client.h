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

#ifndef _EMOTIV_EPOC_HID_CLIENT_H_
#define _EMOTIV_EPOC_HID_CLIENT_H_

#include "emotiv/emotiv_client.h"
#include "emokit/emokit.h"
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <cassert>
#include <boost/thread.hpp>

namespace Emotiv
{

class HIDClient : public Client
{
 public:
  bool update ();
  inline bool init (const char* fname, const char* chapter);
  inline void deinit ();

 protected:
  static void keep_reading (emokit_frame* const data, const bool* const quit);

  int _i;
  bool _quit;
  boost::thread *_thread;
};

};

#endif
