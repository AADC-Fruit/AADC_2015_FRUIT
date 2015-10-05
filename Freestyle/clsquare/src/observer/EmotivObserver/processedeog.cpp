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

#include "processedeog.h"
#include "valueparser.h"
#include <cstring>
#include <fstream>

bool ProcessedEOGServer::init (const int plant_state_dim, const int measurement_dim, const int action_dim, int &observed_state_dim, const char *fname, const char *chapter)
{
  _mdim = measurement_dim;
  observed_state_dim = _mdim + 2;

  // connect
  int port;
  CLS::ValueParser vp(fname, chapter==0 ? "Observer" : chapter);
  vp.get("port", port, 7100);
  try {
    IOUT("Waiting for connection on port " << port << "...");
    _server = new CLS::Util::TCPserver();
    _socket = new CLS::Util::TCPsocket();
    _server->init(port);
    _server->listen();
    _server->accept(*_socket);
    CLS::Util::TCPutils::set_fd_nonblock(_socket->fd);
  } catch (int e) {
    EOUT("Could not create socket.");
    return false;
  }
  IOUT("Connection established.");
 
  _input.str("");
  return true;
}

void ProcessedEOGServer::get_observed_state (const double *prev, const double* action, const double *current, const int cycle, double *observation)
{
  // keep original state
  for (_i=0; _i<_mdim; _i++)
    observation[_i] = current[_i];

  // defaults
  observation[_mdim]   = 0.;
  observation[_mdim+1] = 0.;

  // get TCP message
  _length = _socket->recv_msg(_buffer, MAX_STR_LEN - 1);
  if (_length < 1)
    return;

  if (_length < MAX_STR_LEN - 1) {
    _buffer[_length] = '\0';
  }
  _buffer[MAX_STR_LEN - 1] = '\0';

  _input << _buffer;
  _input_string = _input.str();

  _pos = _input_string.find('\n');
  if (_pos != std::string::npos) {
    std::pair<size_t, size_t> datapoint_indices = find_last_datapoint();

    _input.clear();
    _input.str("");
    _input << _input_string.substr(datapoint_indices.first, datapoint_indices.second - datapoint_indices.first);

    _input >> observation[_mdim];
    _input >> observation[_mdim+1];

    // check sanity of data
    assert(observation[_mdim]   >=  0.);
    assert(observation[_mdim]   <=  1.);
    assert(observation[_mdim+1] >= -1.);
    assert(observation[_mdim+1] <=  1.);

    _input.clear();
    _input.str("");

    // Assure that we read the messages completely
    assert(_input_string.size() == datapoint_indices.second + 1);
  }
}

std::pair<size_t, size_t> ProcessedEOGServer::find_last_datapoint()
{
  size_t end_of_last_data_point = _input_string.rfind("\n");
  size_t start_of_last_data_point = _input_string.rfind("\n", end_of_last_data_point - 1);
  if (start_of_last_data_point == std::string::npos) {
    start_of_last_data_point = 0;
  } else {
    start_of_last_data_point++;
  }
  return std::pair<size_t, size_t>(start_of_last_data_point, end_of_last_data_point);
}

void ProcessedEOGServer::deinit ()
{
  if (_socket != NULL) {
    _socket->close();
    delete _socket;
    _socket = NULL;
  }

  if (_server != NULL) {
    _server->close();
    delete _server;
    _server = NULL;
  }
}

REGISTER(Observer, ProcessedEOGServer, "Adds EOG data from an Emotiv EPOC headset.");
