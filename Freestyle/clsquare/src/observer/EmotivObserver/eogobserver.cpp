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

#include "eogobserver.h"
#include "emotiv/pipe_client.h"
#include "emotiv/hid_client.h"
#include "emotiv/tcp_client.h"
#include "valueparser.h"
#include "utils/filter.h"
#include <cstring>
#include <fstream>
#include "ArithmeticObserver/calcobserver.h"

using namespace CLS::Util::FilterCollection;

bool EOGObserver::init (const int plant_state_dim, const int measurement_dim, const int action_dim, int &observed_state_dim, const char *fname, const char *chapter)
{
  CLS::ValueParser vp(fname, chapter==NULL ? "Observer" : chapter);
  _mdim = measurement_dim;
  observed_state_dim = _mdim + 2;

  // choose connection mode
  char con[255];
  sprintf(con, "%s", "pipe");
  vp.get("connection", con, 255);
  if (strcmp(con, "pipe") == 0) {
    IOUT("Connecting to EPOC via pipe. Please start the server program.");
    _emo = new Emotiv::PipeClient();
  } else if (strcmp(con, "hid") == 0) {
    IOUT("Connecting to EPOC directly.");
    _emo = new Emotiv::HIDClient();
  } else if (strcmp(con, "tcp") == 0) {
    IOUT("Connecting to EPIC via TCP/IP. Please start the client program.");
    _emo = new Emotiv::TCPClient();
  } else {
    EOUT("Unknown connection type \"" << con << "\".");
    return false;
  }

  // connect to device
  if (!_emo->init(fname, chapter==NULL ? "Controller" : chapter)) {
    EOUT("Could not connect to device.");
    return false;
  }

  // config path
  if (vp.get("config", _config, 255) < 1) {
    EOUT("No configuration file specified.");
    return false;
  }

  _quit = false;
  bool ica;
  vp.get("ica", ica, false);
  _thread = new boost::thread(ica ? check_for_event_ica : check_for_event_manual, _emo, &_event, &_quit, _config);

  return true;
}

void EOGObserver::deinit ()
{
  if (_emo != NULL) {
    _emo->deinit();
    delete _emo;
  }
  _emo = NULL;

  delete _thread;
}

void EOGObserver::get_observed_state (const double *prev_measurement, const double* prev_action, const double *current_measurement, const int cycle_ctr, double *observed_state)
{
  if (_quit) {
    CLSERR("Data acquisition thread appears to have terminated prematurely.");
  }

  // keep original state
  for (_i=0; _i<_mdim; _i++)
    observed_state[_i] = current_measurement[_i];

  // split event components
  switch (_event) {
   default:
    observed_state[_mdim]   = 0.;
    observed_state[_mdim+1] = 0.;
    break;
   case EOG_SWITCH:
    observed_state[_mdim]   = 1.;
    observed_state[_mdim+1] = 0.;
    break;
   case EOG_NEXT: 
    observed_state[_mdim]   = 0.;
    observed_state[_mdim+1] = 1.;
    break;
   case EOG_PREV:
    observed_state[_mdim]   = 0.;
    observed_state[_mdim+1] = -1.;
    break;
   case EOG_LOCK:
    observed_state[_mdim]   = 0.1;
    observed_state[_mdim+1] = 0.1;
    break;
  }
  _event = 0;
}

void EOGObserver::check_for_event_ica (Emotiv::Client* const emo, int* const event, const bool* const quit, const char* const fname)
{
  IOUT("Using ICA channel mixing mode.");
  int i;

  // init filters
  std::vector<Filter*> highpass, lowpass;
  HighpassFilter hp; hp.init(fname, "Highpass");
  LowpassFilter  lp; lp.init(fname, "Lowpass");
  for (i=0; i<14; i++) {
    highpass.push_back(hp.clone());
    lowpass.push_back(lp.clone());
  }    
  HysteresisLockFilter lock_blink, lock_direction;
  ChangeFilter change_blink, change_direction;
  lock_blink.init(fname, "Blink");
  lock_direction.init(fname, "Direction");
  change_blink.init(fname, "Change");
  change_direction.init(fname, "Change");

  // load transformation matrices
  Eigen::MatrixXd in_vec(1,14);
  Eigen::MatrixXd center(1,14);
  Eigen::MatrixXd KW(14,2);
  Eigen::MatrixXd out_vec;
  init_matrix_from_file(fname, "center", center, 1, 14);
  init_matrix_from_file(fname, "kw",     KW,     14, 2);

  // init rest
  CLS::ValueParser vp(fname, "Main");
  double signal[16];
  *event = 0;
  int pause;
  vp.get("pause", pause, 10000);
  int channel_dir, channel_switch;
  bool swap;
  vp.get("swap_channels", swap, false);
  if (swap) {
    channel_dir = 0;
    channel_switch = 1;
  } else {
    channel_dir = 1;
    channel_switch = 0;
  }

  // logging
  char logpath[500];
  bool log = false;
  std::ofstream logfile;
  if (vp.get("logfile", logpath, 500) > 1) {
    logfile.open(logpath);
    log = true;
  }

  bool first = true;
  while (!*quit) {
    emo->update();
    emokit_frame current = emo->get_data();
    emo->get_channels<emokit_frame, double>(current, signal);

    // initialize highpass
    if (first) {
      for (i=0; i<2000; i++)
        for (int j=0; j<14; j++) {
          highpass[j]->process(signal[j]);
      }
      first = false;
    }

    // apply pre-mixing filters
    for (i=0; i<14; i++) {
      signal[i] = highpass[i]->process(signal[i]);
      signal[i] = lowpass[i]->process(signal[i]);
      in_vec(0,i) = signal[i];
    }

    // apply ICA mixer
    out_vec = (in_vec - center) * KW;

    // apply post-mixing filters
    bool locked = false;
#define FILTER_ICA_CHANNEL(_channel_,_filter_) \
    signal[_channel_+14] = out_vec(0, _channel_); \
    signal[_channel_+14] = lock_##_filter_.process(signal[_channel_+14]); \
locked |= signal[_channel_+14] != 0.; \
    signal[_channel_+14] = change_##_filter_.process(signal[_channel_+14]);
    FILTER_ICA_CHANNEL(channel_dir, direction);
    FILTER_ICA_CHANNEL(channel_switch, blink);

    if (log) {
      for (i=0; i<16; i++)
        logfile << signal[i] << " ";
      logfile << std::endl;
    }

    // check for event
    // TODO prevents us from using thresholds under 1; should check for thresh here
    if (*event == 0) {
      if (signal[14+channel_switch] >= 1. || signal[14+channel_switch] <= -1)
        *event = EOG_SWITCH;
      else if (signal[14+channel_dir] >= 1.)
        *event = EOG_NEXT;
      else if (signal[14+channel_dir] <= -1.)
        *event = EOG_PREV;
      else if (locked)
        *event = EOG_LOCK;
    }

    usleep(pause);
  }

  if (log)
    logfile.close();
}

void EOGObserver::check_for_event_manual (Emotiv::Client* const emo, int* const event, const bool* const quit, const char* const fname)
{
  IOUT("Using manual channel mixing mode.");

  // init filters
  HighpassFilter hp_blink, hp_direction;
  LowpassFilter lp_blink, lp_direction;
  HysteresisLockFilter lock_blink, lock_direction;
  ChangeFilter change_blink, change_direction;
  hp_blink.init(fname, "Highpass");
  hp_direction.init(fname, "Highpass");
  lp_blink.init(fname, "Lowpass");
  lp_direction.init(fname, "Lowpass");
  lock_blink.init(fname, "Blink");
  lock_direction.init(fname, "Direction");
  change_blink.init(fname, "Change");
  change_direction.init(fname, "Change");

  // init rest
  double signal[16];
  double action[1] = {0};
  *event = 0;
  int pause;
  CLS::ValueParser vp(fname, "Main");
  vp.get("pause", pause, 10000);

  // mix channels
  ArithmeticObserver combi;
  int tmp;
  if (!combi.init(14, 14, 1, tmp, fname, "Channels") || tmp != 16) {
    EOUT("Could not initialize channel mixer");
    return;
  }

  bool first = true;
  while (!*quit) {

    emokit_frame current = emo->get_data();
    emo->get_channels<emokit_frame, double>(current, signal);
    combi.get_observed_state(signal, action, signal, 0, signal);

    // initialize highpass
    if (first) {
      for (int i=0; i<1000; i++) {
        hp_direction.process(signal[14]);
        hp_blink.process(signal[15]);
      }
      first = false;
    }

    // apply post filters
#define FILTER_CHANNEL(_channel_,_filter_) \
    signal[_channel_] = hp_##_filter_.process(signal[_channel_]); \
    signal[_channel_] = lp_##_filter_.process(signal[_channel_]); \
    signal[_channel_] = lock_##_filter_.process(signal[_channel_]); \
    signal[_channel_] = change_##_filter_.process(signal[_channel_]);
    FILTER_CHANNEL(14, direction);
    FILTER_CHANNEL(15, blink);

    // check for event
    if (*event == 0) {
      if (signal[15] > 1.)
        *event = EOG_SWITCH;
      else if (signal[14] > 1.)
        *event = EOG_NEXT;
      else if (signal[14] < -1.)
        *event = EOG_PREV;
    }

    usleep(pause);
  }
}

bool EOGObserver::init_matrix_from_file (const char* cfgname, const char* entry, Eigen::MatrixXd& matrix, const int rows, const int cols)
{
  CLS::ValueParser vp(cfgname, "ICA");
  char filename[255];
  if (vp.get(entry, filename, 255) < 1) {
    CLSERR("Cannot load transformation matrix. No parameter \"" << entry << "\" defined in file " << cfgname << ", chapter [ICA].");
    return false;
  }

  std::ifstream file(filename);
  if (!file.good()) {
    CLSERR("File " << filename << " could not be opened.");
    return false;
  }

  double value;
  for (int r=0; r<rows; r++)
    for (int c=0; c<cols; c++) {
      if (!file.good()) {
        CLSERR("File " << filename << " does not contain a valid description for a " << rows << "x" << cols << " matrix.");
        return false;
      }
      file >> value;
      matrix(r,c) = value;
  }

  return true;
}

REGISTER(Observer, EOGObserver, "Adds EOG data from an Emotiv EPOC headset.");

