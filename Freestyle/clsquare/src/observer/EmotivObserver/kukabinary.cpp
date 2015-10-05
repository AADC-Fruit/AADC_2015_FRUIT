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

#include "kukabinary.h"
#include "valueparser.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>

/**
 * Handles state input:
 * state[0] - x-position of the object
 * state[1] - y-position of the object
 * state[2] - blink (value >= 1)
 */
bool KukaBinaryController::get_action (const double *state, double *action)
{
  // blink detected
  if (fabs(state[2]) >= 1.) {
    // Attempts to send the corresponding value
    try {
      // Reset the buffer
      for (_i = 0;  _i < 4;  _i++) {
        _buffer[_i] = '\0';
      }

      IOUT("Receive state[0]=" << state[0]);
      char output_character = '#';
      
      // Check which command to send
      if (state[0] == 100) {
        // Case: Left
        output_character = 'l';
      } else if (state[0] == 290) {
        // Case: Straight
        output_character = 's';
      } else if (state[0] == 480) {
        // Case: Right
        output_character = 'r';
      } else if (state[0] == 670) {
        // Case: Delete
        output_character = 'x';
      }

      // Print the corresponding values to the buffer
      sprintf(&_buffer[0], "%c", output_character);
      
      // Print a debug message
      IOUT("Sending " << _buffer);
      
      // Send the actual value
      _socket->sendTo((void*)&_buffer[0], 3, _host, _port);

      // Old code
      //_socket->send((void*)&_buffer[0], 3);
    } catch (CLS::Util::SocketException e) {
      EOUT("Failed to send data: " << e.what());
      return false;
    }
  }
  return true;
}

bool KukaBinaryController::init (const int odim, const int adim, double dt, const char *fname, const char* chapter)
{
  if (odim != 3) {
    EOUT("Controller supports only three-dimensional states.");
    return false;
  }

  CLS::ValueParser vp(fname, chapter==NULL ? "Controller" : chapter);
  //char address[25] = "172.0.0.1";
  //vp.get("address", address, 25);
  _host = new char[25];
  vp.get("port", _port, 7009);
  vp.get("host", _host, 25, "127.0.0.1");

  IOUT("Port: " << _port);
  IOUT("Host: " << _host);

  try {
    IOUT("Creating server on port " << _port << ".");
    //_server = new CLS::Util::TCPServerSocket(port);
    _socket = new CLS::Util::UDPSocket();
    IOUT("UDP socket successfully created");    

    //IOUT("Listening for incoming connections.");
    //_socket = _server->accept();
  } catch (CLS::Util::SocketException e) {
    EOUT("Could not create socket: " << e.what());
    return false;
  }
  IOUT("Connection established.");
  return true;
}

void KukaBinaryController::deinit ()
{
  /*if (_server != NULL)
    delete _server;
  _server = NULL;*/
  delete[] _host;
}

REGISTER(Controller, KukaBinaryController, "Foobar")

