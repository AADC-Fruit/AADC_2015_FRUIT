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
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

#include "emotiv/tcp_client.h"
#include "emotiv/global.h"
#include "emotiv/valueparser.h"
#include <string.h>
#include <errno.h>

namespace Emotiv
{

std::pair<size_t, size_t> TCPClient::find_last_datapoint() 
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

bool TCPClient::update ()
{
  _length = _socket->recv_msg(_buffer, EMOTIV_BUFFER_SIZE - 1);
  if (_length < EMOTIV_BUFFER_SIZE - 1) {
    _buffer[_length] = '\0';
  }
  _buffer[EMOTIV_BUFFER_SIZE - 1] = '\0';
IOUT(_buffer);

  _input << _buffer;
  _input_string = _input.str();
    
  _pos = _input_string.find('\n');
  if (_pos != std::string::npos) { 
    std::pair<size_t, size_t> datapoint_indices = find_last_datapoint();

    _input.clear();
    _input.str("");
    _input << _input_string.substr(datapoint_indices.first, 
                                   datapoint_indices.second - datapoint_indices.first);

    /// \todo could use these values to check sanity of data
    for (_i=0; _i<4; _i++)
      _input >> _device_data[_i];
    _data.counter = _device_data[0];
    _data.battery = _device_data[1];
    _data.gyroX = _device_data[2];
    _data.gyroY = _device_data[3];

    for (_i=0; _i<EMOTIV_SENSOR_DIM; ++_i) {
      _input >> _sensor_data[_i];
      _input >> _contact_quality[_i];
    }
    Client::set_channels<emokit_frame,int>(_data,_sensor_data);
    Client::set_channels<emokit_contact_quality,int>(_data.cq,_contact_quality);

    _input.clear();
    _input.str("");

    // Assure that we read the messages completely
    assert(_input_string.size() == datapoint_indices.second + 1);
  }

  return true;
}

bool TCPClient::init (const char* fname, const char* chapter)
{
  _input.str("");

  int port;
  ValueParser vp(fname, chapter);
  vp.get("port", port, 7100);

  try {
    IOUT("Waiting for client to connect on port " << port << "...");
    _server = new TCPserver();
    _socket = new TCPsocket();
    _server->init(port);
    _server->listen();
    _server->accept(*_socket);
    TCPutils::set_fd_nonblock(_socket->fd);
  } catch (int e) {
    EOUT("Could not create socket.");
    return false;
  }

  IOUT("Connection established.");
  return true;
}

void TCPClient::deinit ()
{
  _socket->close();
  _server->close();
}

};
