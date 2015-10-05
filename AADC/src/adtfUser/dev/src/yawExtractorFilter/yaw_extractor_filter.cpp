#include "stdafx.h"
#include "yaw_extractor_filter.h"
#include <iostream>

// -------------------------------------------------------------------------------------------------
ADTF_FILTER_PLUGIN("FRUIT Yaw Extractor Filter", OID_ADTF_YAW_EXTRACTOR_FILTER, YawExtractorFilter);
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
YawExtractorFilter::YawExtractorFilter(const tChar* __info) : cFilter(__info) {    
// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------
YawExtractorFilter::~YawExtractorFilter() {
// -------------------------------------------------------------------------------------------------
}


// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::CreateInputPins(__exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(input_qW_.Create("quaternion_gyro_w", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink*> (this)));
  RETURN_IF_FAILED(RegisterPin(&input_qW_));

  RETURN_IF_FAILED(input_qX_.Create("quaternion_gyro_x", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink*> (this)));
  RETURN_IF_FAILED(RegisterPin(&input_qX_));

  RETURN_IF_FAILED(input_qY_.Create("quaternion_gyro_y", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink*> (this)));
  RETURN_IF_FAILED(RegisterPin(&input_qY_));

  RETURN_IF_FAILED(input_qZ_.Create("quaternion_gyro_z", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink*> (this)));
  RETURN_IF_FAILED(RegisterPin(&input_qZ_));

  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::CreateOutputPins(__exception) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaDescriptionManager> pDescManager;
  RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager,__exception_ptr));
  
  tChar const * strDescSignalValue = pDescManager->GetMediaDescription("tSignalValue");
  RETURN_IF_POINTER_NULL(strDescSignalValue);        
  cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValue,IMediaDescription::MDF_DDL_DEFAULT_VERSION);    
  RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSignal)); 
  
  RETURN_IF_FAILED(output_yaw_radians_.Create("yaw_rad", pTypeSignalValue, NULL));
  RETURN_IF_FAILED(RegisterPin(&output_yaw_radians_));

  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::TransmitAngles(tTimeStamp sampleTimeStamp, const tTimeStamp timeStampValue, const tFloat32 yaw_radians, const tFloat32 yaw_degrees) {    
// -------------------------------------------------------------------------------------------------
  //create new media sample
  cObjectPtr<IMediaSample> pMediaSampleYawRad;
  //cObjectPtr<IMediaSample> pMediaSampleYawDeg;
  AllocMediaSample((tVoid**)&pMediaSampleYawRad);
  //AllocMediaSample((tVoid**)&pMediaSampleYawDeg);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  m_pCoderDescSignal->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSampleYawRad->AllocBuffer(nSize);
  //pMediaSampleYawDeg->AllocBuffer(nSize);
  
  {
    __adtf_sample_write_lock_mediadescription(m_pCoderDescSignal, pMediaSampleYawRad, coder);
    coder->Set("f32Value", (tVoid*) &yaw_radians);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStampValue);
  }

  //transmit media sample over output pin
  pMediaSampleYawRad->SetTime(sampleTimeStamp);
  output_yaw_radians_.Transmit(pMediaSampleYawRad);
  
  //write date to the media sample with the coder of the descriptor
  //m_pCoderDescSignal->WriteLock(pMediaSampleYawDeg, &pCoderOutput);    
  //pCoderOutput->Set("f32Value", (tVoid*)&(yaw_degrees));    
  //pCoderOutput->Set("ui32ArduinoTimestamp", (tVoid*)&timeStampValue);    
  //m_pCoderDescSignal->Unlock(pCoderOutput);
  //transmit media sample over output pin
  //pMediaSampleYawDeg->SetTime(sampleTimeStamp);
  //output_yaw_degrees_.Transmit(pMediaSampleYawDeg);

  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))

  if (eStage == StageFirst) {
    CreateInputPins(__exception_ptr);
    CreateOutputPins(__exception_ptr);
  }else if (eStage == StageGraphReady) {
    qX = .0;
    qY = .0;
    qZ = .0;
    qW = .0;
  }
      
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::Start(__exception) {
// -------------------------------------------------------------------------------------------------
  return cFilter::Start(__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::Stop(__exception) {
// -------------------------------------------------------------------------------------------------
  return cFilter::Stop(__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  return cFilter::Shutdown(eStage,__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::calculateYaw(tFloat32 &yaw_radians, tFloat32 &yaw_degrees) {
// -------------------------------------------------------------------------------------------------
  //double roll = atan2(2*qY*qW - 2*qX*qZ, 1 - 2*qY*qY - 2*qZ*qZ);
  double pitch = atan2(2*qX*qW - 2*qY*qZ, 1 - 2*qX*qX - 2*qZ*qZ);
  double yaw = asin(2*qX*qY + 2*qZ*qW);
  
  if(fabs(pitch) < M_PI / 2.0) {
    yaw_radians = -yaw;
    yaw_degrees = -yaw * 180.0 / M_PI;
  } else {
    if(yaw < 0) {
      yaw_radians = M_PI + yaw;
      yaw_degrees = yaw_radians * 180.0 / M_PI; 
    } else {
      yaw_radians = -M_PI + yaw;
      yaw_degrees = yaw_radians * 180.0 / M_PI;
    }
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult YawExtractorFilter::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample) {
// -------------------------------------------------------------------------------------------------
  if (nEventCode == IPinEventSink::PE_MediaSampleReceived && pMediaSample != NULL && m_pCoderDescSignal != NULL) {
    RETURN_IF_POINTER_NULL( pMediaSample);    
    tUInt32 timeStamp = 0;
    if (pSource == &input_qW_) {    
      {
        __adtf_sample_read_lock_mediadescription(m_pCoderDescSignal, pMediaSample, coder);
        coder->Get("f32Value", (tVoid*) &qW);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
      }      
    }
    else if (pSource == &input_qX_) {
      {
        __adtf_sample_read_lock_mediadescription(m_pCoderDescSignal, pMediaSample, coder);
        coder->Get("f32Value", (tVoid*) &qX);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
      }
    }
    else if (pSource == &input_qY_) {
      {
        __adtf_sample_read_lock_mediadescription(m_pCoderDescSignal, pMediaSample, coder);
        coder->Get("f32Value", (tVoid*) &qY);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
      }
    }
    else if (pSource == &input_qZ_) {
      {
        __adtf_sample_read_lock_mediadescription(m_pCoderDescSignal, pMediaSample, coder);
        coder->Get("f32Value", (tVoid*) &qZ);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
      }
      
      //right order to spell
      tFloat32 yaw_radians, yaw_degrees;
      
      calculateYaw(yaw_radians, yaw_degrees);
      TransmitAngles(pMediaSample->GetTime(), timeStamp, yaw_radians, yaw_degrees);      
    }            
  }
  
  RETURN_NOERROR;
}
