/**
 *
 * ADTF Template Project
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: belkera $
 * $Date: 2011-06-30 16:51:21 +0200 (Thu, 30 Jun 2011) $
 * $Revision: 26514 $
 *
 * @remarks
 *
 */
#include "stdafx.h"
#include "moose.h"
#include <template_data.h>
#include <fstream>

#define THRESHOLD 30

/// Create filter shell
ADTF_FILTER_PLUGIN("Moose Filter", OID_ADTF_MOOSE_FILTER, MooseFilter);


MooseFilter::MooseFilter(const tChar* __info):cFilter(__info)
{
    LOG_INFO("Constructed");
}

MooseFilter::~MooseFilter()
{

}

tResult MooseFilter::Init(tInitStage eStage, __exception)
{
    LOG_INFO("Init yo");
    // never miss calling the parent implementation!!
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))
    
    // Copy-pasted code from Carvisualization.cpp that processes sensor data. --------------------------------------------------------------------------
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager,__exception_ptr));
        
    //input descriptor
    tChar const * strDescSignalValue = pDescManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strDescSignalValue);        
    cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValue,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSignalInput));

    // End of copypasta ---------------------------------------------------------------------------------------------------------------------------------

    // in StageFirst you can create and register your static pins.
    if (eStage == StageFirst)
    {        
        // ----- Input pins

        // get a media type for the input pin
        cObjectPtr<IMediaType> pInputType;
        RETURN_IF_FAILED(AllocMediaType(&pInputType, MEDIA_TYPE_TEMPLATE, MEDIA_SUBTYPE_TEMPLATE, __exception_ptr));
        
        // create and register the input pin
        // front-left US-sensor
        RETURN_IF_FAILED(input_uss_front_left_.Create("input_uss_front_left", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink*>(this)));
        RETURN_IF_FAILED(RegisterPin(&input_uss_front_left_));
        // front-right US-sensor
        RETURN_IF_FAILED(input_uss_front_right_.Create("input_uss_front_right", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink*>(this)));
        RETURN_IF_FAILED(RegisterPin(&input_uss_front_right_));

        // -----
        // Output pins
        // -----
        tChar const * strDescBoolSignalValue = pDescManager->GetMediaDescription("tBoolSignalValue");
        RETURN_IF_POINTER_NULL(strDescBoolSignalValue);        
        cObjectPtr<IMediaType> pTypeBoolSignalValueOutput = new cMediaType(0, 0, 0, "tBoolSignalValue", strDescBoolSignalValue,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
		RETURN_IF_FAILED(pTypeBoolSignalValueOutput->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescBoolSignalOutput));

        tChar const * strDescSignalValueOutput = pDescManager->GetMediaDescription("tSignalValue");
        RETURN_IF_POINTER_NULL(strDescSignalValueOutput);        
        cObjectPtr<IMediaType> pTypeSignalValueOutput = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValueOutput,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
        RETURN_IF_FAILED(pTypeSignalValueOutput->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSignalOutput));

        // Headlight Output
		RETURN_IF_FAILED(output_head_lights_.Create("output_head_lights", pTypeBoolSignalValueOutput, NULL));
		RETURN_IF_FAILED(RegisterPin(&output_head_lights_));

        // Turn Signal Left
        RETURN_IF_FAILED(output_turn_signal_left_.Create("output_turn_signal_left", pTypeBoolSignalValueOutput, NULL));
		RETURN_IF_FAILED(RegisterPin(&output_turn_signal_left_));
        
        // Turn Signal Right
        RETURN_IF_FAILED(output_turn_signal_right_.Create("output_turn_signal_right", pTypeBoolSignalValueOutput, NULL));
		RETURN_IF_FAILED(RegisterPin(&output_turn_signal_right_));
    
        // Breaklight Output
        RETURN_IF_FAILED(output_break_lights_.Create("output_break_lights", pTypeBoolSignalValueOutput, NULL));
		RETURN_IF_FAILED(RegisterPin(&output_break_lights_));

        // Motor Output
        RETURN_IF_FAILED(output_motors_.Create("output_motors", pTypeSignalValueOutput, NULL));
		RETURN_IF_FAILED(RegisterPin(&output_motors_));

        // Steer Angle Output
		RETURN_IF_FAILED(output_steering_angle_.Create("output_steering_angle", pTypeSignalValueOutput, NULL));
        RETURN_IF_FAILED(RegisterPin(&output_steering_angle_)); 
    }
    else if (eStage == StageNormal)
    {
        // In this stage you would do further initialisation and/or create your dynamic pins.
        // Please take a look at the demo_dynamicpin example for further reference.
    }
    else if (eStage == StageGraphReady)
    {
        // All pin connections have been established in this stage so you can query your pins
        // about their media types and additional meta data.
        // Please take a look at the demo_imageproc example for further reference.
    }
    
    // Initialize default values.
    uss_front_left_value_ = uss_front_right_value_ = motor_speed_ = steering_angle_ = 0.0; 
    head_lights_value_ = break_lights_value_ = left_signal_value_ = right_signal_value_ = false;
    cycle_ = 0;
    RETURN_NOERROR;
}

tResult MooseFilter::Shutdown(tInitStage eStage, __exception)
{
    LOG_INFO("Shutdown");
    // In each stage clean up everything that you initiaized in the corresponding stage during Init.
    // Pins are an exception: 
    // - The base class takes care of static pins that are members of this class.
    // - Dynamic pins have to be cleaned up in the ReleasePins method, please see the demo_dynamicpin
    //   example for further reference.
    
    if (eStage == StageGraphReady)
    {
    }
    else if (eStage == StageNormal)
    {
    }
    else if (eStage == StageFirst)
    {
    }

    // call the base class implementation
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult MooseFilter::OnPinEvent(IPin* pSource,
                                           tInt nEventCode,
                                           tInt nParam1,
                                           tInt nParam2,
                                           IMediaSample* pMediaSample)
{
    //LOG_INFO(adtf_util::cString::Format("### PIN EVENT ###"));
    // first check what kind of event it is
    // Annotation by lohmannm: when using the filter, it appeared that only every second
    // pin event is actually a media sample. That is why there will always be two lines
    // of "### PIN EVENT ###" between two sets of data.
    if (nEventCode == IPinEventSink::PE_MediaSampleReceived)
    {
        tFloat32 signalValue = 0;
        tUInt32 timeStamp = 0;
        // so we received a media sample, so this pointer better be valid.
        RETURN_IF_POINTER_NULL(pMediaSample);
        if (pMediaSample != NULL && m_pCoderDescSignalInput != NULL)
                {
                // read-out the incoming Media Sample
                cObjectPtr<IMediaCoder> pCoderInput;
                RETURN_IF_FAILED(m_pCoderDescSignalInput->Lock(pMediaSample, &pCoderInput));
                
                //get values from media sample        
                pCoderInput->Get("f32Value", (tVoid*)&signalValue);
                pCoderInput->Get("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
                m_pCoderDescSignalInput->Unlock(pCoderInput); 
                //LOG_INFO(adtf_util::cString::Format("Signal: %f", signalValue));
                //LOG_INFO(adtf_util::cString::Format("TimeStamp: %d", timeStamp));         
                }                
            else
			    RETURN_ERROR(ERR_FAILED);

        if (pSource == &input_uss_front_left_) {
            uss_front_left_value_ = signalValue;
        }
        else if (pSource == &input_uss_front_right_) {
            uss_front_right_value_ = signalValue;        
        }

        // Obstacle in front. STOP.
        if (uss_front_left_value_ < THRESHOLD && uss_front_right_value_ < THRESHOLD) {
            SetLightConfig(cycle_, false, true, false, false);
            MotorSpeed(0);
            SteeringAngle(0);            
        }
        // Obstacle left. Turn right.
        else if (uss_front_left_value_ < THRESHOLD && uss_front_right_value_ >= THRESHOLD) {
            SetLightConfig(cycle_, true, false, false, true);
            MotorSpeed(15);
            SteeringAngle(60);
        }
        // Obstacle right. Turn left.
        else if (uss_front_left_value_ >= THRESHOLD && uss_front_right_value_ < THRESHOLD) {
            SetLightConfig(cycle_, true, false, true, false);
            MotorSpeed(15);
            SteeringAngle(-60);
        }
        // No obstacle. Drive.
        else if (uss_front_left_value_ >= THRESHOLD && uss_front_right_value_ >= THRESHOLD) {
            SetLightConfig(cycle_, true, false, false, false);
            MotorSpeed(20);
            SteeringAngle(0);
        }
        cycle_++;
        LOG_INFO(cString::Format("%d. cycle", cycle_));
        if (cycle_ == 4) cycle_ = 0;
    }
    RETURN_NOERROR;
}

tResult MooseFilter::TransmitLight(unsigned int lightIndex, tBool state)
{
    //write values with zero
    tUInt32 timeStamp = 0;
	
    //create new media sample
    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid**)&pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    m_pCoderDescBoolSignalOutput->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pMediaSample->AllocBuffer(nSize);
       
    //write date to the media sample with the coder of the descriptor
    cObjectPtr<IMediaCoder> pCoderOutput;
    m_pCoderDescBoolSignalOutput->WriteLock(pMediaSample, &pCoderOutput);	

    pCoderOutput->Set("bValue", (tVoid*)&(state));	
    pCoderOutput->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);    
    m_pCoderDescBoolSignalOutput->Unlock(pCoderOutput);

    //transmit media sample over output pin
    pMediaSample->SetTime(_clock->GetStreamTime());
    
    switch(lightIndex)
    {
        case 0:
            output_head_lights_.Transmit(pMediaSample);
            head_lights_value_ = state;
            break;
        case 1:
            output_break_lights_.Transmit(pMediaSample);
            break_lights_value_ = state;
            break;
        case 2:
            output_turn_signal_left_.Transmit(pMediaSample);
            left_signal_value_ = state;
            break;
        case 3:
            output_turn_signal_right_.Transmit(pMediaSample);
            right_signal_value_ = state;            
            break;
    }
    
    LOG_INFO(cString::Format("Setting light %d", lightIndex));
    
	RETURN_NOERROR;
}

tResult MooseFilter::SetLightConfig(int cycle, tBool head, tBool breaklight, tBool left, tBool right)
{
    LOG_INFO(cString::Format("%d. cycle in method", cycle));
    if (cycle == 0/* && head != head_lights_value_*/) TransmitLight(0, head);
    if (cycle == 1/*breaklight != break_lights_value_*/) TransmitLight(1, breaklight);
    if (cycle == 2/*left != left_signal_value_*/) TransmitLight(2, left);
    if (cycle == 3/*right != right_signal_value_*/) TransmitLight(3, right);

    RETURN_NOERROR;
}

void MooseFilter::MotorSpeed(int val)
{
    //if (val == motor_speed_) return;
    //motor_speed_ = val;
	tFloat32 flValue= (tFloat32)(val);
	tUInt32 timeStamp = 0;
							
    //create new media sample
    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid**)&pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    m_pCoderDescSignalOutput->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pMediaSample->AllocBuffer(nSize);
       
    //write date to the media sample with the coder of the descriptor
    cObjectPtr<IMediaCoder> pCoder;
    m_pCoderDescSignalOutput->WriteLock(pMediaSample, &pCoder);	
		
	pCoder->Set("f32Value", (tVoid*)&(flValue));	
    pCoder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);    
    m_pCoderDescSignalOutput->Unlock(pCoder);
    
    //transmit media sample over output pin
    pMediaSample->SetTime(_clock->GetStreamTime());
    output_motors_.Transmit(pMediaSample);

    LOG_INFO(cString::Format("Setting motor speed to %d", val));
}

void MooseFilter::SteeringAngle(int val)
{
    //if (val == steering_angle_) return;
    //steering_angle_ = val;

    tFloat32 flValue= ((tFloat32)val)*35.f/((tFloat32)100);
	tUInt32 timeStamp = 0;
					
	//LOG_INFO(cString::Format("slot speed: %f",flValue));	
	
    //create new media sample
    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid**)&pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    m_pCoderDescSignalOutput->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pMediaSample->AllocBuffer(nSize);
       
    //write date to the media sample with the coder of the descriptor
    cObjectPtr<IMediaCoder> pCoder;
    m_pCoderDescSignalOutput->WriteLock(pMediaSample, &pCoder);	
		
	pCoder->Set("f32Value", (tVoid*)&(flValue));	
    pCoder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);    
    m_pCoderDescSignalOutput->Unlock(pCoder);
    
    //transmit media sample over output pin
    pMediaSample->SetTime(_clock->GetStreamTime());
    output_steering_angle_.Transmit(pMediaSample);

    LOG_INFO(cString::Format("Setting steering angle to %d", val));
}
