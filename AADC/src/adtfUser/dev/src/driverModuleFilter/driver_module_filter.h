/**
Copyright (c) 
Audi Autonomous Driving Cup. All rights reserved.
 
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: “This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.”
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


**********************************************************************
* $Author:: forchhe#$  $Date:: 2014-09-11 10:10:54#$ $Rev:: 25921   $
* $Author:: wormo#$    $Date:: 2015-03-04 15:30:00#$ $Rev:: 90001   $
* $Author:: riestern#$ $Date:: 2015-03-04 15:30:00#$ $Rev:: 90001   $
**********************************************************************/


#ifndef _DRIVER_FILTER_HEADER
#define _DRIVER_FILTER_HEADER

#define OID_ADTF_DRIVER_MODULE_FILTER "fruit.dev.driver_module_filter"

#include "stdafx.h"
#include <vector>
#include <time.h>
#include "maneuverlist.h"

class DriverModuleFilter : public cFilter {
  ADTF_FILTER(OID_ADTF_DRIVER_MODULE_FILTER, "FRUIT Driver Module Filter", adtf::OBJCAT_DataFilter);

 public:
  DriverModuleFilter(const tChar*);
  virtual ~DriverModuleFilter();

  tResult Init(tInitStage eStage, __exception = NULL);
  tResult Start(__exception = NULL);
  tResult Stop(__exception = NULL);
  tResult Shutdown(tInitStage eStage, __exception = NULL);
  tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);
  tResult Run(tInt nActivationCode, const tVoid* pvUserData, tInt szUserDataSize, ucom::IException** __exception_ptr=NULL);
    
private:
  tResult sendFlag(bool value, tTimeStamp timeStamp);
  tResult OnSendState(tInt8 i8StateID, tInt16 i16ManeuverEntry);
  tResult sendManeuver(int maneuver_id, tTimeStamp timeStamp);
  bool isTimeOkay();
  cString getAction(int maneuver_id);
  int getManeuverEncoding(cString maneuver);

  cObjectPtr<IMediaTypeDescription> m_pCoderDescJuryStruct;
  cObjectPtr<IMediaTypeDescription> m_pCoderDescDriverStruct;
  cObjectPtr<IMediaTypeDescription> flag_description_;
  cObjectPtr<IMediaTypeDescription> signal_description_;

  cInputPin m_JuryStructInputPin;
  cInputPin maneuver_complete_input_;
  cOutputPin maneuver_output_;
  cOutputPin m_DriverStructOutputPin;
  cOutputPin flag_output_;
  
  tBool m_bDebugModeEnabled;

  cFilename m_maneuverListFile;
  tResult loadManeuverList();
  std::vector<tSector> m_sectorList;
  
  bool current_flag_;
  time_t start_time_;
  bool is_time_done_;
  
  int current_maneuver_;
  int max_maneuver_id_;
};

#endif
