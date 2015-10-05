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
#include "stdafx.h"
#include "driver_module_filter.h"
#include "../util/maneuver_actions.h"
#include "juryEnums.h"
#include <unistd.h>
#include <iostream>

ADTF_FILTER_PLUGIN("FRUIT Driver Module Filter", OID_ADTF_DRIVER_MODULE_FILTER, DriverModuleFilter)

// -------------------------------------------------------------------------------------------------
DriverModuleFilter::DriverModuleFilter(const tChar* __info) : cFilter(__info), m_bDebugModeEnabled(false) {
// -------------------------------------------------------------------------------------------------
  SetPropertyBool("debug", false);
  SetPropertyStr("ManeuverFile", "");
  SetPropertyBool("ManeuverFile" NSSUBPROP_FILENAME, tTrue);
  SetPropertyStr("ManeuverFile" NSSUBPROP_FILENAME NSSUBSUBPROP_EXTENSIONFILTER, "XML Files (*.xml)");
  SetPropertyFloat("Cruise control delay", 10.0f);
}

// -------------------------------------------------------------------------------------------------
DriverModuleFilter::~DriverModuleFilter() {
// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

  // pins need to be created at StageFirst
  if (eStage == StageFirst)    {
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER, (tVoid**)&pDescManager, __exception_ptr));

    // input jury struct
    tChar const * strDesc1 = pDescManager->GetMediaDescription("tJuryStruct");
    RETURN_IF_POINTER_NULL(strDesc1);
    cObjectPtr<IMediaType> pType1 = new cMediaType(0, 0, 0, "tJuryStruct", strDesc1, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(m_JuryStructInputPin.Create("Jury_Struct", pType1, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_JuryStructInputPin));
    RETURN_IF_FAILED(pType1->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescJuryStruct));
    
    // output driver struct
    tChar const * strDesc2 = pDescManager->GetMediaDescription("tDriverStruct");
    RETURN_IF_POINTER_NULL(strDesc2);
    cObjectPtr<IMediaType> pType2 = new cMediaType(0, 0, 0, "tDriverStruct", strDesc2, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(m_DriverStructOutputPin.Create("Driver_Struct", pType2, NULL));
    RETURN_IF_FAILED(RegisterPin(&m_DriverStructOutputPin));
    RETURN_IF_FAILED(pType2->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescDriverStruct));
    
    // Create the flag media description
    tChar const * flagValue = pDescManager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(flagValue);
    cObjectPtr<IMediaType> flag_value_type = new cMediaType(0, 0, 0, "tBoolSignalValue", flagValue, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(flag_value_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&flag_description_));
    
    // Create the flag output pin
    RETURN_IF_FAILED(flag_output_.Create("flag_output", flag_value_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&flag_output_));
    
    // Create the maneuver complete input pin
    RETURN_IF_FAILED(maneuver_complete_input_.Create("Maneuver_Complete", flag_value_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&maneuver_complete_input_));
    
    // Create the maneuver output description
    tChar const * signal_value = pDescManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(signal_value);
    cObjectPtr<IMediaType> signal_value_type = new cMediaType(0, 0, 0, "tSignalValue", signal_value, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(signal_value_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&signal_description_));
    
    // Create the maneuver output pin
    RETURN_IF_FAILED(maneuver_output_.Create("Maneuver_Out", signal_value_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&maneuver_output_));
    
    m_bDebugModeEnabled = GetPropertyBool("debug");
  }
  else if(eStage == StageGraphReady) {
    loadManeuverList();
    current_flag_ = false;
    time(&start_time_);
    is_time_done_ = false;
    current_maneuver_ = -1;
  }
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::Start(__exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Start(__exception_ptr));
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::Run(tInt nActivationCode, const tVoid* pvUserData, tInt szUserDataSize, ucom::IException** __exception_ptr/* =NULL */) {
// -------------------------------------------------------------------------------------------------
  return cFilter::Run(nActivationCode, pvUserData, szUserDataSize, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::Stop(__exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Stop(__exception_ptr));    
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::Shutdown(tInitStage eStage, __exception) { 
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Shutdown(eStage, __exception_ptr));
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_POINTER_NULL(pMediaSample);
  RETURN_IF_POINTER_NULL(pSource);

  if (nEventCode == IPinEventSink::PE_MediaSampleReceived) { 
    if (pSource == &maneuver_complete_input_) {
      bool update_counter = false;
      tTimeStamp timeStamp = 0;
      
      {
        __adtf_sample_read_lock_mediadescription(flag_description_, pMediaSample, coder);
        coder->Get("bValue", (tVoid*) &update_counter);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
      }
      
      if (update_counter) {
        // Increment the maneuver id
        current_maneuver_++;
        
        // Check, whether the incremented id is valid
        if (current_maneuver_ > max_maneuver_id_) {
          // Send the complete state and stop the car
          OnSendState(stateCar_COMPLETE, current_maneuver_ - 1);
          sendFlag(0, timeStamp);
        } else {
          // Send the running signal for the new maneuver and the corresponding action
          OnSendState(stateCar_RUNNING, current_maneuver_);
          sendManeuver(current_maneuver_, timeStamp);
        }
      }
    }
         
    else if (pSource == &m_JuryStructInputPin) {
      tInt8 i8ActionID = -2;
      tInt16 i16entry = -1;
      
      {
        __adtf_sample_read_lock_mediadescription(m_pCoderDescJuryStruct,pMediaSample,pCoder);
        pCoder->Get("i8ActionID", (tVoid*) &i8ActionID);
        pCoder->Get("i16ManeuverEntry", (tVoid*) &i16entry);              
      }

      if (i8ActionID == action_STOP) {
        if(m_bDebugModeEnabled)  LOG_INFO(cString::Format("Driver Module: Received: Stop with maneuver ID %d",i16entry));
        // Send the flag with value false, because the car should stop
        if (current_flag_ == true) {
        std::cout << "Received stop command!" << std::endl;
          OnSendState(stateCar_ERROR, current_maneuver_);
          sendFlag(false, pMediaSample->GetTime());
          current_flag_ = false;
        }
      }
      
      else if (i8ActionID == action_GETREADY) {
        if(m_bDebugModeEnabled)  LOG_INFO(cString::Format("Driver Module: Received: Request Ready with maneuver ID %d", i16entry));
        if (!isTimeOkay()) {
          OnSendState(stateCar_STARTUP, i16entry);
          time_t now;
          time(&now);
          double diff_sec = difftime(now, start_time_);
          usleep((GetPropertyFloat("Cruise control delay") - diff_sec + 1) * 1000 * 1000);
        }
        
        current_maneuver_ = i16entry;
        OnSendState(stateCar_READY, current_maneuver_);
      }
      
      else if (i8ActionID == action_START) {
        if(m_bDebugModeEnabled)  LOG_INFO(cString::Format("Driver Module: Received: Run with maneuver ID %d",i16entry));
        // Send the flag with value true, because the car is now allowed to go!
        if (current_flag_ == false && isTimeOkay()) {
          current_maneuver_ = i16entry;
          OnSendState(stateCar_RUNNING, current_maneuver_);
          sendManeuver(current_maneuver_, pMediaSample->GetTime());
          sendFlag(true, pMediaSample->GetTime());
          current_flag_ = true;
        }
      }
    }
  }
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::sendManeuver(int maneuver_id, tTimeStamp timeStamp) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  signal_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  
  cString maneuver = getAction(maneuver_id);
  if (m_bDebugModeEnabled) LOG_INFO(cString::Format("Send maneuver: %s", maneuver.GetPtr()));
  
  int action_encoding = getManeuverEncoding(maneuver);
  if (action_encoding == -1) {
    LOG_ERROR(cString::Format("An error has occurred while encoding: %s", maneuver.GetPtr()));
    RETURN_NOERROR;
  }
  
  {
    __adtf_sample_write_lock_mediadescription(signal_description_, media_sample, coder);
    coder->Set("f32Value", (tVoid*) &action_encoding);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
  }
  
  maneuver_output_.Transmit(media_sample);
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
int DriverModuleFilter::getManeuverEncoding(cString maneuver) {
// -------------------------------------------------------------------------------------------------
  if (maneuver.Compare(cString("straight")) == 0) return STRAIGHT;
  else if (maneuver.Compare(cString("left")) == 0) return LEFT;
  else if (maneuver.Compare(cString("right")) == 0) return RIGHT;
  else if (maneuver.Compare(cString("parallel_parking")) == 0) return PARALLEL_PARKING;
  else if (maneuver.Compare(cString("cross_parking")) == 0) return CROSS_PARKING;
  else if (maneuver.Compare(cString("pull_out_left")) == 0) return PULL_OUT_LEFT;
  else if (maneuver.Compare(cString("pull_out_right")) == 0) return PULL_OUT_RIGHT;
  
  return -1;
}

// -------------------------------------------------------------------------------------------------
cString DriverModuleFilter::getAction(int maneuver_id) {
// -------------------------------------------------------------------------------------------------
  for (size_t i = 0; i < m_sectorList.size(); i++) {
    for (size_t j = 0; j < m_sectorList.at(i).maneuverList.size(); j++) {
      if (m_sectorList.at(i).maneuverList.at(j).id == maneuver_id) {
        return m_sectorList.at(i).maneuverList.at(j).action;
      }
    }
  }
  
  return cString("Invalid");
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::OnSendState(tInt8 i8StateID, tInt16 i16ManeuverEntry) {            
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> pMediaSample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&pMediaSample));

  cObjectPtr<IMediaSerializer> pSerializer;
  m_pCoderDescDriverStruct->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();

  RETURN_IF_FAILED(pMediaSample->AllocBuffer(nSize));
  
  {
    __adtf_sample_write_lock_mediadescription(m_pCoderDescDriverStruct,pMediaSample,pCoder);
    pCoder->Set("i8StateID", (tVoid*)&i8StateID);
    pCoder->Set("i16ManeuverEntry", (tVoid*)&i16ManeuverEntry);
  }      

  pMediaSample->SetTime(_clock->GetStreamTime());
  m_DriverStructOutputPin.Transmit(pMediaSample);
  
  if(m_bDebugModeEnabled) {
    LOG_INFO(cString::Format("Driver Module: Send: state: %d, Maneuver ID %d",i8StateID,i16ManeuverEntry));
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::loadManeuverList() {
// -------------------------------------------------------------------------------------------------
  m_maneuverListFile = GetPropertyStr("ManeuverFile");

  if (m_maneuverListFile.IsEmpty()) {
    LOG_ERROR("DriverFilter: Maneuver file not found");
    RETURN_ERROR(ERR_INVALID_FILE);
  }    

  ADTF_GET_CONFIG_FILENAME(m_maneuverListFile);

  m_maneuverListFile = m_maneuverListFile.CreateAbsolutePath(".");

  //Load file, parse configuration, print the data
  if (cFileSystem::Exists(m_maneuverListFile)) {
    cDOM oDOM;
    oDOM.Load(m_maneuverListFile);        
    cDOMElementRefList oSectorElems;
    cDOMElementRefList oManeuverElems;

    //read first Sector Elem
    if(IS_OK(oDOM.FindNodes("AADC-Maneuver-List/AADC-Sector", oSectorElems))) {                
      //iterate through sectors
      for (cDOMElementRefList::iterator itSectorElem = oSectorElems.begin(); itSectorElem != oSectorElems.end(); ++itSectorElem) {
        //if sector found
        tSector sector;
        sector.id = (*itSectorElem)->GetAttributeUInt32("id");
        
        if(IS_OK((*itSectorElem)->FindNodes("AADC-Maneuver", oManeuverElems))) {
          //iterate through maneuvers
          for(cDOMElementRefList::iterator itManeuverElem = oManeuverElems.begin(); itManeuverElem != oManeuverElems.end(); ++itManeuverElem) {
            tAADC_Maneuver man;
            man.id = (*itManeuverElem)->GetAttributeUInt32("id");
            man.action = (*itManeuverElem)->GetAttribute("action");
            sector.maneuverList.push_back(man);
          }
        }

        m_sectorList.push_back(sector);
      }
    }
    if (oSectorElems.size() > 0) {
      LOG_INFO("DriverFilter: Loaded Maneuver file successfully.");
    }
    else {
      LOG_ERROR("DriverFilter: no valid Maneuver Data found!");
      RETURN_ERROR(ERR_INVALID_FILE);
    }
  }
  
  else {
    LOG_ERROR("DriverFilter: no valid Maneuver File found!");
    RETURN_ERROR(ERR_INVALID_FILE);
  }
  
  // Calculate max id
  max_maneuver_id_ = -1;
  for (size_t i = 0; i < m_sectorList.size(); i++) {
    max_maneuver_id_ += m_sectorList.at(i).maneuverList.size();
  }

  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult DriverModuleFilter::sendFlag(bool value, tTimeStamp timeStamp) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  flag_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  
  {
    __adtf_sample_write_lock_mediadescription(flag_description_, media_sample, coder);
    coder->Set("bValue", (tVoid*) &value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
  }
  
  if (m_bDebugModeEnabled) LOG_INFO(cString::Format("Send flag: %d", value));
  flag_output_.Transmit(media_sample);
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
bool DriverModuleFilter::isTimeOkay() {
// -------------------------------------------------------------------------------------------------
  if (is_time_done_) return true;
  
  time_t now;
  time(&now);
  double diff_sec = difftime(now, start_time_);
  if (diff_sec > GetPropertyFloat("Cruise control delay")) {
    is_time_done_ = true;
    if (m_bDebugModeEnabled) LOG_INFO("Time delay reached.");
    return true;
  }
  
  return false;
}
