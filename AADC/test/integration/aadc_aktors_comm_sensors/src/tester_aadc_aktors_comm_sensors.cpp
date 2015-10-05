/**
 *
 * AADC Arduino Aktors filter tests
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: VG8D3AW $
 * $Date: 2013-02-06 16:30:41 +0100 (Mi, 06. Feb 2013) $
 * $Revision: 18162 $
 *
 * @remarks
 *
 */

#include "stdafx.h"
#include "tester_aadc_aktors_comm_sensors.h"

IMPLEMENT_TESTER_CLASS(cTesterAADCAktorsCommSensors,
                "1",
                "AADC Arduino Aktors filter",
                "This test makes sure that the AADC Arduino Aktors filter works as expected",
                "");


/*
Setup for this test
*/
void cTesterAADCAktorsCommSensors::setUp()
{
    // create the environment
    SERVICE_ENV_SETUP;

    // minimum needed ADTF services
    SERVICE_ENV_ADD_PLUGIN("adtf_clock.srv");
    SERVICE_ENV_ADD_PLUGIN("adtf_kernel.srv");
    SERVICE_ENV_ADD_PLUGIN("adtf_sample_pool.srv");
    SERVICE_ENV_ADD_PLUGIN("adtf_namespace.srv");
    SERVICE_ENV_ADD_PLUGIN("adtf_media_description.srv");

    // register services
    SERVICE_ENV_REGISTER_SERVICE(OID_ADTF_REFERENCE_CLOCK, "referenceclock", IRuntime::RL_Kernel);
    SERVICE_ENV_REGISTER_SERVICE(OID_ADTF_KERNEL, "kernel", IRuntime::RL_System);
    SERVICE_ENV_REGISTER_SERVICE(OID_ADTF_SAMPLE_POOL, "samplepool", IRuntime::RL_System);
    SERVICE_ENV_REGISTER_SERVICE(OID_ADTF_NAMESPACE, "namespace", IRuntime::RL_System);
    SERVICE_ENV_REGISTER_SERVICE(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, "mediadesc", IRuntime::RL_System);

    // add filter plugin
    FILTER_ENV_ADD_PLUGIN("aadc_aktors.plb");
    FILTER_ENV_ADD_PLUGIN("aadc_arduinoCom.plb");
    FILTER_ENV_ADD_PLUGIN("aadc_sensors.plb");
    FILTER_ENV_ADD_PLUGIN("aadc_watchdogTrigger.plb");

    // set runlevel system
    SERVICE_ENV_SET_RUNLEVEL(IRuntime::RL_System);

    // set the path to the MediaDescription-Files
    // (relative to the current working dir which is the tester dir)
    cFilename strDescrFileADTF = "../../../src/adtfBase/AADC_ADTF_BaseFilters/description/aadc.description";
    cFilename strDescrFile = "../../../src/adtfBase/AADC_ADTF_BaseFilters/description/aadc.description";

    // create absolute path
    strDescrFileADTF = strDescrFileADTF.CreateAbsolutePath(cFileSystem::GetCurDirectory());
    strDescrFile = strDescrFile.CreateAbsolutePath(cFileSystem::GetCurDirectory());

    // get the media description manager
    cObjectPtr<IMediaDescriptionManager> pMediaDesc;
    __adtf_test_result_ext(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER, (tVoid**) &pMediaDesc),
        "unable to get media description manager");

    // get the config from the media description manager
    cObjectPtr<IConfiguration> pConfig;
    __adtf_test_result(pMediaDesc->GetInterface(IID_ADTF_CONFIGURATION, (tVoid**) &pConfig));

    // set the path to the description files
    __adtf_test_result(pConfig->SetPropertyStr("media_description_files", cString::Format("%s;%s", strDescrFileADTF.GetPtr(), strDescrFile.GetPtr())));

    // check if the media description manager loads the description correctly
    __adtf_test(pMediaDesc->GetMediaDescription("tArduinoData") != NULL);

    // set runlevel application
    SERVICE_ENV_SET_RUNLEVEL(IRuntime::RL_Application);
}

/*
Tear down for this test
*/
void cTesterAADCAktorsCommSensors::tearDown()
{
    // give the kernel some time to quit its threads
    //cSystem::Sleep(200000);
    FILTER_ENV_TEAR_DOWN;
}

/************************************************************************/
/* This is just a helper class to receive data from output pins.                                                                     */
/************************************************************************/
class cMediaSampleSink: public IPinEventSink
{
    // ucom helper macro
    UCOM_OBJECT_IMPL(IID_ADTF_PIN_EVENT_SINK, adtf::IPinEventSink);
public:
    // constructor
    cMediaSampleSink(): m_ui32SampleCount(0)
    { 
    }

    // destructor
    virtual ~cMediaSampleSink()
    {
    }

    // members to count and hold the received media samples
    tUInt32 m_ui32SampleCount;
    std::vector<cObjectPtr<IMediaSample> > m_vecMediaSamples;

public:
    tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
    {
        if (nEventCode == IPinEventSink::PE_MediaSampleTransmitted)
        {
            // count the samples
            ++m_ui32SampleCount;
            // push sample into vector for later compare
            m_vecMediaSamples.push_back(pMediaSample);
        }

        RETURN_NOERROR;
    }

};

/************************************************************************/
/* These are just helper functions to send Media Samples.                                                                     */
/************************************************************************/
tTestResult transmitSignalMediaSample(cOutputPin *pSampleSource, const tFloat32 f32Value )
{
    // get the mediatype of the sample source
    cObjectPtr<IMediaType> pType;
    if(IS_FAILED(pSampleSource->GetMediaType(&pType)))
    {
        __adtf_test_ext(tFalse, "unable to get mediatype");
    }

    // get the type description
    cObjectPtr<IMediaTypeDescription> pTypeDesc;
    pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&pTypeDesc);

    // get the serializer from description to get the deserialized size
    cObjectPtr<IMediaSerializer> pSerializer;
    pTypeDesc->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pSerializer = NULL;
   
    // create and allocate the sample
    cObjectPtr<IMediaSample> pSample;
    _runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**)&pSample);
    pSample->AllocBuffer(nSize);

    // get the coder
    cObjectPtr<IMediaCoder> pCoder;
    pTypeDesc->WriteLock(pSample, &pCoder);

    // use the coder to set the value
    pCoder->Set("f32Value", &f32Value);
 
    // unlock the coder
    pTypeDesc->Unlock(pCoder);
    // set the sample time (in this case the time doesn't matter and must not be the stream time)
    pSample->SetTime(0);
    // transmit the media sample
    //__adtf_test_result(pSampleSource->Transmit(pSample));
    if(IS_FAILED(pSampleSource->Transmit(pSample)))
    {
        __adtf_test_ext(tFalse, "unable to send media sample");
    }

}
tTestResult transmitBoolMediaSample(cOutputPin *pSampleSource, const tBool bValue)
{
    // get the mediatype of the sample source
    cObjectPtr<IMediaType> pType;
    if(IS_FAILED(pSampleSource->GetMediaType(&pType)))
    {
        __adtf_test_ext(tFalse, "unable to get mediatype");
    }

    // get the type description
    cObjectPtr<IMediaTypeDescription> pTypeDesc;
    pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&pTypeDesc);

    // get the serializer from description to get the deserialized size
    cObjectPtr<IMediaSerializer> pSerializer;
    pTypeDesc->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pSerializer = NULL;    

    // create and allocate the sample
    cObjectPtr<IMediaSample> pSample;
    _runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**)&pSample);
    pSample->AllocBuffer(nSize);

    // get the coder
    cObjectPtr<IMediaCoder> pCoder;
    pTypeDesc->WriteLock(pSample, &pCoder);

    // use the coder to write the value in the sample
    pCoder->Set("bValue", &bValue);
 
    // unlock the coder
    pTypeDesc->Unlock(pCoder);
    // set the sample time (in this case the time doesn't matter and must not be the stream time)
    pSample->SetTime(0);
    // transmit the media sample
    //__adtf_test_result(pSampleSource->Transmit(pSample));
    if(IS_FAILED(pSampleSource->Transmit(pSample)))
    {
        __adtf_test_ext(tFalse, "unable to send media sample");
    }

}
tTestResult transmitEmergencyMediaSample(cOutputPin *pSampleSource, const tBool bValue)
{
    // get the mediatype of the sample source
    cObjectPtr<IMediaType> pType;
    if(IS_FAILED(pSampleSource->GetMediaType(&pType)))
    {
        __adtf_test_ext(tFalse, "unable to get mediatype");
    }

    // get the type description
    cObjectPtr<IMediaTypeDescription> pTypeDesc;
    pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&pTypeDesc);

    // get the serializer from description to get the deserialized size
    cObjectPtr<IMediaSerializer> pSerializer;
    pTypeDesc->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pSerializer = NULL;    

    // create and allocate the sample
    cObjectPtr<IMediaSample> pSample;
    _runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**)&pSample);
    pSample->AllocBuffer(nSize);

    // get the coder
    cObjectPtr<IMediaCoder> pCoder;
    pTypeDesc->WriteLock(pSample, &pCoder);

    // use the coder to write the value in the sample
    pCoder->Set("bEmergencyStop", (tVoid*)&bValue);
 
    // unlock the coder
    pTypeDesc->Unlock(pCoder);
    // set the sample time (in this case the time doesn't matter and must not be the stream time)
    pSample->SetTime(0);
    // transmit the media sample
    //__adtf_test_result(pSampleSource->Transmit(pSample));
    if(IS_FAILED(pSampleSource->Transmit(pSample)))
    {
        __adtf_test_ext(tFalse, "unable to send media sample");
    }

}


/************************************************************************/
/* These are just helper functions to evaluate Media Samples.                                                                     */
/************************************************************************/
tTestResult evaluateSteerAngle(cMediaSampleSink *pSampleSink)
{
    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescSteerAngle = pDescManager->GetMediaDescription("tSteeringAngleData");
    __adtf_test_pointer(strDescSteerAngle);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeSteerAngleData = new cMediaType(0, 0, 0, "tSteeringAngleData", strDescSteerAngle,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSteerAngleData;
    __adtf_test_result(pTypeSteerAngleData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSteerAngleData));

    // received sensor data should show the actor ramp
    // now check the received data

    // Define the Ranges of the ramp and a tolerance Value to ignore single value jumps
    tUInt16 ui16LowerRampRange = 338; 
    tUInt16 ui16UpperRampRange = 520; 
    tUInt16 ui16ToleranceValue = 20;

    // Init some values we need to recognize the ramp
    tUInt16 ui16SteerAngleBefore = 0;
    tBool bStartOfRamp = tFalse;
    tInt8 i8RampSegment = 10;
    tUInt16 ui16StartAngle = 0;

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values 
        tUInt16 ui16Angle = 0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        pCoder->Get("ui16Angle", (tVoid*)&ui16Angle);
        pCoder->Get("ui32ArduinoTimestamp", (tVoid*)&ui32ArduinoTimestamp);

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        // test the received data
        __adtf_test(ui32ArduinoTimestamp != 0);

        __adtf_test_log(cString::Format("Steerangle: %u", ui16Angle));
        __adtf_test_log(cString::Format("Rampsegment: %d", i8RampSegment));

        // Test for the ramp
        // recognize initial state
        if(ui16Angle == 0)
        {
            i8RampSegment = 0;            
        }
        // recognize the start of the ramp
        if(!bStartOfRamp && ui16Angle > 0 && i8RampSegment == 0)
        {
            ui16StartAngle = ui16Angle;
            bStartOfRamp = tTrue;
            i8RampSegment++;
        }
        // recognize the first turn
        if(bStartOfRamp && ui16Angle > ui16UpperRampRange && i8RampSegment == 1)
        {
            i8RampSegment++;
            bStartOfRamp = tFalse;
        }
        // recognize the second turn
        if(bStartOfRamp && ui16Angle < ui16LowerRampRange && i8RampSegment == 2)
        {
            i8RampSegment++;
            bStartOfRamp = tFalse;
        }
        // recognize the end of the ramp
        if(bStartOfRamp && ui16Angle > ui16StartAngle - ui16ToleranceValue  && i8RampSegment == 3)
        {
            i8RampSegment++;
            bStartOfRamp = tFalse;
        }
        // start the ramp recognition again after a turn if the values are in the range
        if(!bStartOfRamp && ui16Angle < ui16UpperRampRange && ui16Angle > ui16LowerRampRange && i8RampSegment < 4)
        {
            bStartOfRamp = tTrue;
        }

        // test the values in the rising ramps
        if(bStartOfRamp && (i8RampSegment == 1 || i8RampSegment == 3))
        {
            __adtf_test(ui16Angle > ui16SteerAngleBefore - ui16ToleranceValue);
        }
        // test the values of the falling ramp
        if(bStartOfRamp && i8RampSegment == 2)
        {
            __adtf_test(ui16Angle < ui16SteerAngleBefore + ui16ToleranceValue);
        }

        // Go to the next Values
        ui16SteerAngleBefore = ui16Angle;
    }
    // Check if the ramp was recognized
    __adtf_test(i8RampSegment == 4);
}
tTestResult evaluateRPM(cMediaSampleSink *pSampleSink, const tChar chFrameId)
{
    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescRPM = pDescManager->GetMediaDescription("tWheelEncoderData");
    __adtf_test_pointer(strDescRPM);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeSteerAngleData = new cMediaType(0, 0, 0, "tWheelEncoderData", strDescRPM,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSteerAngleData;
    __adtf_test_result(pTypeSteerAngleData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSteerAngleData)); 

    // received sensor data should show the actor ramp
    // now check the received data

    // Define the Ranges of the ramp and a tolerance Value to ignore single value jumps
    tUInt16 ui16LowerRampRange = 3; 
    tUInt16 ui16UpperRampRange = 21;
    tUInt16 ui16ToleranceValue = 4;

    // Init some values we need to recognize the ramp
    tUInt32 ui32RPMDiffBefore = 0;
    tUInt32 ui32RPMBefore = 0;
    tBool bStartOfRamp = tFalse;
    tInt8 i8RampSegment = 0;
    tUInt16 ui16EndRPMDiff = 12;

    // for the combination test take smaller values
    if(chFrameId == 0xff)
    {
            ui16UpperRampRange = 11;
            ui16EndRPMDiff = 3;
    }

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // only take every third value to receive stronger differences
        if(nIdx % 2)
        {
            // get the coder
            cObjectPtr<IMediaCoder> pCoder;
            m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
            // init the values 
            tUInt32 ui32RPMLeft = 0;
            tUInt32 ui32RPMRight = 0;
            tUInt32 ui32ArduinoTimestamp = 0;

            // get values from coder
            pCoder->Get("ui32LeftWheel", (tVoid*)&ui32RPMLeft);
            pCoder->Get("ui32RightWheel", (tVoid*)&ui32RPMRight);
            pCoder->Get("ui32ArduinoTimestamp", (tVoid*)&ui32ArduinoTimestamp);

            // unlock 
            m_pCoderDescSteerAngleData->Unlock(pCoder);

            // test the received data
            __adtf_test(ui32ArduinoTimestamp!=0);

            // build the average between left and right wheel
            tUInt32 ui32RPM = (ui32RPMLeft + ui32RPMRight)/2;

            // weigth the new value statistically with the old one to filter out peak values
            ui32RPM = (ui32RPMBefore + ui32RPM)/2;

            __adtf_test_log(cString::Format("RPM: %d", ui32RPM - ui32RPMBefore));
            __adtf_test_log(cString::Format("Rampsegment: %d", i8RampSegment));

            // Test for the ramp
            // recognize the start of the ramp
            if(!bStartOfRamp && ui32RPM - ui32RPMBefore > 0 && i8RampSegment == 0)
            {
                bStartOfRamp = tTrue;
                i8RampSegment++;
            }
            // recognize the first turn
            if(bStartOfRamp && ui32RPM - ui32RPMBefore > ui16UpperRampRange && (i8RampSegment == 1 || i8RampSegment == 3))
            {
                i8RampSegment++;
                bStartOfRamp = tFalse;
            }
            // recognize the second turn
            if(bStartOfRamp && ui32RPM - ui32RPMBefore < ui16LowerRampRange && i8RampSegment == 2)
            {
                i8RampSegment++;
                bStartOfRamp = tFalse;
            }
            // recognize the end of the ramp
            if(bStartOfRamp && ui32RPM - ui32RPMBefore < ui16EndRPMDiff && i8RampSegment == 4)
            {
                i8RampSegment++;
                bStartOfRamp = tFalse;
            }
            // start the ramp recognition again after a turn if the values are in the range
            if(!bStartOfRamp && ui32RPM - ui32RPMBefore < ui16UpperRampRange && ui32RPM - ui32RPMBefore > ui16LowerRampRange && i8RampSegment < 4)
            {
                bStartOfRamp = tTrue;
            }

            // test the values in the rising ramps
            if(bStartOfRamp && (i8RampSegment == 1 || i8RampSegment == 3))
            {
                __adtf_test(ui32RPM - ui32RPMBefore + ui16ToleranceValue > ui32RPMDiffBefore);
            }
            // test the values of the falling ramp
            if(bStartOfRamp && i8RampSegment == 2)
            {
                __adtf_test(ui32RPM - ui32RPMBefore < ui32RPMDiffBefore + ui16ToleranceValue);
            }

            // Go to the next Values
            ui32RPMDiffBefore = ui32RPM - ui32RPMBefore;
            ui32RPMBefore = ui32RPM;

        }
    }

    // Check if the ramp was recognized
    __adtf_test(i8RampSegment == 4);
}

tTestResult DoActuatorCommSensorTest(const tChar chFrameId, tInt nSleep = 0)
{
    // initialize filter
    INIT_FILTER(pAktFilter, pAktFilterConfig, "adtf.aadc.aktors");
    INIT_FILTER(pCommFilter, pCommFilterConfig, "adtf.aadc.arduinoCOM");
    INIT_FILTER(pSensFilter, pSensFilterConfig, "adtf.aadc.sensors");
    INIT_FILTER(pWDFilter, pWDFilterConfig, "adtf.aadc.watchdogGuard");

#ifdef WIN32 // WIN32 is also defined on WIN64
    // set the COM-Port on the Windows machine (on ODROID we use the default setting from filter)
    __adtf_test_result(pCommFilterConfig->SetPropertyStr("COM Port", " \\\\.\\COM9"));
#endif

    // set filter to state ready
    SET_STATE_READY(pAktFilter);
    SET_STATE_READY(pCommFilter);
    SET_STATE_READY(pSensFilter);
    SET_STATE_READY(pWDFilter);

    // get the output pin of the WD Filter
    cObjectPtr<IPin> pWDOutPin;
    __adtf_test_result_ext(pWDFilter->FindPin("WatchdogAliveSignal", IPin::PD_Output, &pWDOutPin), "Unable to find pin WatchdogAliveSignal");

    // get the input pins of the Aktors Filter
    cObjectPtr<IPin> pAccInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("accelerate", IPin::PD_Input, &pAccInPin), "Unable to find input pin of Aktors filter");
    cObjectPtr<IPin> pSteerAngleInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("steerAngle", IPin::PD_Input, &pSteerAngleInPin), "Unable to find input pin of Aktors filter");
    cObjectPtr<IPin> pHeadLightInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("headLightEnabled", IPin::PD_Input, &pHeadLightInPin), "Unable to find input pin of Aktors filter");
    cObjectPtr<IPin> pTurnLeftInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("turnSignalLeftEnabled", IPin::PD_Input, &pTurnLeftInPin), "Unable to find input pin of Aktors filter");
    cObjectPtr<IPin> pTurnRightInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("turnSignalRightEnabled", IPin::PD_Input, &pTurnRightInPin), "Unable to find input pin of Aktors filter");
    cObjectPtr<IPin> pBrakeLightInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("brakeLightEnabled", IPin::PD_Input, &pBrakeLightInPin), "Unable to find input pin of Aktors filter");
    cObjectPtr<IPin> pReverseLightInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("reverseLightsEnabled", IPin::PD_Input, &pReverseLightInPin), "Unable to find input pin of Aktors filter");
    cObjectPtr<IPin> pWDInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("Watchdog_Alive_Flag", IPin::PD_Input, &pWDInPin), "Unable to find input pin of Aktors filter");
    cObjectPtr<IPin> pEmergencyInPin;
    __adtf_test_result_ext(pAktFilter->FindPin("Emergency_Stop", IPin::PD_Input, &pEmergencyInPin), "Unable to find input pin of Aktors filter");

    // get the output pin of the Aktors Filter
    cObjectPtr<IPin> pArdAktOutPin;
    __adtf_test_result_ext(pAktFilter->FindPin("ArduinoCOM_output", IPin::PD_Output, &pArdAktOutPin), "Unable to find pin ArduinoCOM_output");

    // get the input pin of the Communication Filter
    cObjectPtr<IPin> pArdCommInPin;
    __adtf_test_result_ext(pCommFilter->FindPin("COM_input", IPin::PD_Input, &pArdCommInPin), "Unable to find input pin of Communication Filter");

    // get the output pin of the Communication Filter
    cObjectPtr<IPin> pArdCommOutPin;
    __adtf_test_result_ext(pCommFilter->FindPin("COM_output", IPin::PD_Output, &pArdCommOutPin), "Unable to find output pin of Communication Filter");

    // get the input pin of the Sensors Filter
    cObjectPtr<IPin> pArdSensInPin;
    __adtf_test_result_ext(pSensFilter->FindPin("ArduinoCOM_input", IPin::PD_Input, &pArdSensInPin), "Unable to find input pin of Sensors Filter");

    // get the output pins of the Sensors Filter
    cObjectPtr<IPin> pArdSensUSOutPin;
    __adtf_test_result_ext(pSensFilter->FindPin("ultrasonic_sensors", IPin::PD_Output, &pArdSensUSOutPin), "Unable to find output pin of Sensors Filter");
    cObjectPtr<IPin> pArdSensIROutPin;
    __adtf_test_result_ext(pSensFilter->FindPin("infrared_sensors", IPin::PD_Output, &pArdSensIROutPin), "Unable to find output pin of Sensors Filter");
    cObjectPtr<IPin> pArdSensSteerAngleOutPin;
    __adtf_test_result_ext(pSensFilter->FindPin("steering_servo", IPin::PD_Output, &pArdSensSteerAngleOutPin), "Unable to find output pin of Sensors Filter");
    cObjectPtr<IPin> pArdSensAccOutPin;
    __adtf_test_result_ext(pSensFilter->FindPin("accelerometer", IPin::PD_Output, &pArdSensAccOutPin), "Unable to find output pin of Sensors Filter");
    cObjectPtr<IPin> pArdSensGyroOutPin;
    __adtf_test_result_ext(pSensFilter->FindPin("gyroscope", IPin::PD_Output, &pArdSensGyroOutPin), "Unable to find output pin of Sensors Filter");
    cObjectPtr<IPin> pArdSensRPMOutPin;
    __adtf_test_result_ext(pSensFilter->FindPin("wheel_speed_sensor", IPin::PD_Output, &pArdSensRPMOutPin), "Unable to find output pin of Sensors Filter");
    cObjectPtr<IPin> pArdSensVoltageOutPin;
    __adtf_test_result_ext(pSensFilter->FindPin("system_voltage", IPin::PD_Output, &pArdSensVoltageOutPin), "Unable to find output pin of Sensors Filter");

    // create a simple sample source for every input pin
    cObjectPtr<cOutputPin> pAccSampleSource = new cOutputPin();
    __adtf_test_result(pAccSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tSignalValue")));
    cObjectPtr<cOutputPin> pSteerAngleSampleSource = new cOutputPin();
    __adtf_test_result(pSteerAngleSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tSignalValue")));
    cObjectPtr<cOutputPin> pHeadLightSampleSource = new cOutputPin();
    __adtf_test_result(pHeadLightSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tBoolSignalValue")));
    cObjectPtr<cOutputPin> pTurnLeftSampleSource = new cOutputPin();
    __adtf_test_result(pTurnLeftSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tBoolSignalValue")));
    cObjectPtr<cOutputPin> pTurnRightSampleSource = new cOutputPin();
    __adtf_test_result(pTurnRightSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tBoolSignalValue")));
    cObjectPtr<cOutputPin> pBrakeLightSampleSource = new cOutputPin();
    __adtf_test_result(pBrakeLightSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tBoolSignalValue")));
    cObjectPtr<cOutputPin> pReverseLightSampleSource = new cOutputPin();
    __adtf_test_result(pReverseLightSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tBoolSignalValue")));
    cObjectPtr<cOutputPin> pEmergencySampleSource = new cOutputPin();
    __adtf_test_result(pEmergencySampleSource->Create("sampleSource", new cMediaType(0,0,0,"tJuryEmergencyStop")));

    // connect the input pins of the aktors filter with the sample sources
    __adtf_test_result(pAccInPin->Connect(pAccSampleSource));
    __adtf_test_result(pSteerAngleInPin->Connect(pSteerAngleSampleSource));
    __adtf_test_result(pHeadLightInPin->Connect(pHeadLightSampleSource));
    __adtf_test_result(pTurnLeftInPin->Connect(pTurnLeftSampleSource));
    __adtf_test_result(pTurnRightInPin->Connect(pTurnRightSampleSource));
    __adtf_test_result(pBrakeLightInPin->Connect(pBrakeLightSampleSource));
    __adtf_test_result(pReverseLightInPin->Connect(pReverseLightSampleSource));
    __adtf_test_result(pWDInPin->Connect(pWDOutPin));
    __adtf_test_result(pEmergencyInPin->Connect(pEmergencySampleSource));

    // connect the output pin of the Aktors Filter with the input pin of the Communication Filter
    __adtf_test_result(pArdCommInPin->Connect(pArdAktOutPin));

    // connect the output pin of the Communication Filter with the input pin of the Sensors Filter
    __adtf_test_result(pArdSensInPin->Connect(pArdCommOutPin));

    // create a sample sink for every output of the sensors filter
    cObjectPtr<cMediaSampleSink> pUSSampleSink = new cMediaSampleSink();
    cObjectPtr<cMediaSampleSink> pIRSampleSink = new cMediaSampleSink();
    cObjectPtr<cMediaSampleSink> pSteerAngleSampleSink = new cMediaSampleSink();
    cObjectPtr<cMediaSampleSink> pAccSampleSink = new cMediaSampleSink();
    cObjectPtr<cMediaSampleSink> pGyroSampleSink = new cMediaSampleSink();
    cObjectPtr<cMediaSampleSink> pRPMSampleSink = new cMediaSampleSink();
    cObjectPtr<cMediaSampleSink> pVoltageSampleSink = new cMediaSampleSink();
  
    // connect the output pins of the sensors filter with the sample sinks
    __adtf_test_result(pArdSensUSOutPin->RegisterEventSink(pUSSampleSink));
    __adtf_test_result(pArdSensIROutPin->RegisterEventSink(pIRSampleSink));
    __adtf_test_result(pArdSensAccOutPin->RegisterEventSink(pSteerAngleSampleSink));
    __adtf_test_result(pArdSensUSOutPin->RegisterEventSink(pAccSampleSink));
    __adtf_test_result(pArdSensGyroOutPin->RegisterEventSink(pGyroSampleSink));
    __adtf_test_result(pArdSensRPMOutPin->RegisterEventSink(pRPMSampleSink));
    __adtf_test_result(pArdSensVoltageOutPin->RegisterEventSink(pVoltageSampleSink));

    // set the filter state running
    SET_STATE_RUNNING(pAktFilter);
    SET_STATE_RUNNING(pCommFilter);
    SET_STATE_RUNNING(pSensFilter);
    SET_STATE_RUNNING(pWDFilter);

    // init the loop count
    tUInt32 ui32MaxLoopCount = 0;
    // init the wait time
    tUInt32 ui32Waittime = 0;
    
    //#############################################################################################################################################
    //Testing
    //#############################################################################################################################################
    
    // define the Test duration and the Waittime after Watchdog Signal is on
    ui32MaxLoopCount = 250;
    // Dafines the Time from when the Watchdog is active to when the first actuator sample is sent. This is because of the initialisation of the motor controler
    ui32Waittime = 5000;

    #ifdef WIN32 // WIN32 is also defined on WIN64
    // Because of the different execution time on windows and linux there have to be different loop cycle numbers to simulate the same time
    ui32MaxLoopCount = 1000;
    ui32Waittime = 1000;
    #endif

    // init the data range
    tFloat32 f32LowerRange = 0.0f;
    tFloat32 f32UpperRange = 0.0f;

    // define the data range
    if(chFrameId == ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
    {
        f32LowerRange = 65.0f;
        f32UpperRange = 125.0f;

    }else if (chFrameId == ID_ARD_ACT_ACCEL_SERVO){
        f32LowerRange = 0.0f;
        f32UpperRange = 180.0f;
    }
    
    // define the LightCycle
    // Number of lights
    tFloat32 f32NumberOfLights = 5;
    // Loopcount when the lights are activated
    tUInt32 ui32NumberOfCycles = ui32MaxLoopCount/4;

    // calculate the Start of each Lighttest
    tFloat32 f32StartOfHeadlight = ui32NumberOfCycles / f32NumberOfLights * 0;
    tFloat32 f32StartOfBrakelight = ui32NumberOfCycles / f32NumberOfLights * 1;
    tFloat32 f32StartOfTurnleftlight = ui32NumberOfCycles / f32NumberOfLights * 2;
    tFloat32 f32StartOfTurnrightlight = ui32NumberOfCycles / f32NumberOfLights * 3;
    tFloat32 f32StartOfReverselight = ui32NumberOfCycles / f32NumberOfLights * 4;

    //################################################################################################
    // send some samples
    //################################################
    // to give the motor controller time for initialisation send for a short time only wd signal 
    for (tUInt32 nIdx = 0; nIdx < ui32Waittime; nIdx++)
    {   
        // Allways send Watchdog Signal
        //Let the Watchdog send some Signals before starting the test
        cSystem::Sleep(1000);
    }
    //###############################################
    // define a Ramp from one border to init Value  
    if(chFrameId == ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
    {
        f32LowerRange = 95.0f;
        f32UpperRange = 125.0f;

    }else if (chFrameId == ID_ARD_ACT_ACCEL_SERVO){
        f32LowerRange = 90.0f;
        f32UpperRange = 180.0f;
    }
    //####################
    // send the samples according to the specified ramp
    for (tUInt32 nIdx = 0; nIdx < ui32MaxLoopCount/4; nIdx++)
    {   

        // Acceleration test
        if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            // send the sample
            transmitSignalMediaSample(pAccSampleSource, f32Value);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);
        }

        // Steer angle test 
        if(chFrameId==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            // send the sample
            transmitSignalMediaSample(pSteerAngleSampleSource, f32Value);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);
        }

        // Light Test
        if(chFrameId == ID_ARD_ACT_LIGHT_DATA || chFrameId == 0xff)
        {
            // set the value
            tBool bValue = tTrue;
            // send the sample 
            // Divide the loop into equal sections for every Light
            if(nIdx > (f32StartOfHeadlight) && nIdx < (f32StartOfBrakelight) ) transmitBoolMediaSample(pHeadLightSampleSource,bValue);
            if(nIdx > (f32StartOfBrakelight) && nIdx < (f32StartOfTurnleftlight) ) transmitBoolMediaSample(pBrakeLightSampleSource,bValue);
            if(nIdx > (f32StartOfTurnleftlight) && nIdx < (f32StartOfTurnrightlight) ) transmitBoolMediaSample(pTurnLeftSampleSource,bValue);;
            if(nIdx > (f32StartOfTurnrightlight) && nIdx < (f32StartOfReverselight) ) transmitBoolMediaSample(pTurnRightSampleSource,bValue);
            if(nIdx > (f32StartOfReverselight) && nIdx < ui32MaxLoopCount) transmitBoolMediaSample(pReverseLightSampleSource,bValue);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);    
        }
    }
    //###############################################
    // Go the same ramp backwards
    //####################
    // send the samples according to the specified ramp
    for (tUInt32 nIdx = ui32MaxLoopCount/4; nIdx > 0 ; nIdx--)
    {   

        // Acceleration test
        if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            // send the sample
            transmitSignalMediaSample(pAccSampleSource, f32Value);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);
        }

        // Steer angle test 
        if(chFrameId==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            // send the sample
            transmitSignalMediaSample(pSteerAngleSampleSource, f32Value);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);
        }

        // Light Test
        if(chFrameId == ID_ARD_ACT_LIGHT_DATA || chFrameId == 0xff)
        {
            // set the value
            tBool bValue = tFalse;
            // send the sample 
            // Divide the loop into equal sections for every Light
            if(nIdx > (f32StartOfHeadlight) && nIdx < (f32StartOfBrakelight) ) transmitBoolMediaSample(pHeadLightSampleSource,bValue);
            if(nIdx > (f32StartOfBrakelight) && nIdx < (f32StartOfTurnleftlight) ) transmitBoolMediaSample(pBrakeLightSampleSource,bValue);
            if(nIdx > (f32StartOfTurnleftlight) && nIdx < (f32StartOfTurnrightlight) ) transmitBoolMediaSample(pTurnLeftSampleSource,bValue);;
            if(nIdx > (f32StartOfTurnrightlight) && nIdx < (f32StartOfReverselight) ) transmitBoolMediaSample(pTurnRightSampleSource,bValue);
            if(nIdx > (f32StartOfReverselight) && nIdx < ui32MaxLoopCount) transmitBoolMediaSample(pReverseLightSampleSource,bValue);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);    
        }
    }
    //###############################################
    // define a Ramp from init Value to the other border
    if(chFrameId == ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
    {
        f32LowerRange = 65.0f;
        f32UpperRange = 95.0f;

    }else if (chFrameId == ID_ARD_ACT_ACCEL_SERVO){
        f32LowerRange = 0.0f;
        f32UpperRange = 90.0f;
    }
    //####################
    // send the samples according to the specified ramp
    for (tUInt32 nIdx = ui32MaxLoopCount/4; nIdx > 0 ; nIdx--)
    {   

        // Acceleration test
        if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            // send the sample
            transmitSignalMediaSample(pAccSampleSource, f32Value);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);
        }

        // Steer angle test 
        if(chFrameId==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            // send the sample
            transmitSignalMediaSample(pSteerAngleSampleSource, f32Value);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);
        }

        // Light Test
        if(chFrameId == ID_ARD_ACT_LIGHT_DATA || chFrameId == 0xff)
        {
            // set the value
            tBool bValue = tTrue;
            // send the sample 
            // Divide the loop into equal sections for every Light
            if(nIdx > (f32StartOfHeadlight) && nIdx < (f32StartOfBrakelight) ) transmitBoolMediaSample(pHeadLightSampleSource,bValue);
            if(nIdx > (f32StartOfBrakelight) && nIdx < (f32StartOfTurnleftlight) ) transmitBoolMediaSample(pBrakeLightSampleSource,bValue);
            if(nIdx > (f32StartOfTurnleftlight) && nIdx < (f32StartOfTurnrightlight) ) transmitBoolMediaSample(pTurnLeftSampleSource,bValue);;
            if(nIdx > (f32StartOfTurnrightlight) && nIdx < (f32StartOfReverselight) ) transmitBoolMediaSample(pTurnRightSampleSource,bValue);
            if(nIdx > (f32StartOfReverselight) && nIdx < ui32MaxLoopCount) transmitBoolMediaSample(pReverseLightSampleSource,bValue);          
            // wait to control the sendrate
            cSystem::Sleep(nSleep);    
        }
    }
    
    //###############################################
    // Go the same ramp backwards
    //####################
    // send the samples according to the specified ramp
    for (tUInt32 nIdx = 0; nIdx < ui32MaxLoopCount/4; nIdx++)
    {   

        // Acceleration test
        if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            // send the sample
            transmitSignalMediaSample(pAccSampleSource, f32Value);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);
        }

        // Steer angle test 
        if(chFrameId==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            // send the sample
            transmitSignalMediaSample(pSteerAngleSampleSource, f32Value);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);
        }

        // Light Test
        if(chFrameId == ID_ARD_ACT_LIGHT_DATA || chFrameId == 0xff)
        {
            // set the value
            tBool bValue = tFalse;
            // send the sample 
            // Divide the loop into equal sections for every Light
            if(nIdx > (f32StartOfHeadlight) && nIdx < (f32StartOfBrakelight) ) transmitBoolMediaSample(pHeadLightSampleSource,bValue);
            if(nIdx > (f32StartOfBrakelight) && nIdx < (f32StartOfTurnleftlight) ) transmitBoolMediaSample(pBrakeLightSampleSource,bValue);
            if(nIdx > (f32StartOfTurnleftlight) && nIdx < (f32StartOfTurnrightlight) ) transmitBoolMediaSample(pTurnLeftSampleSource,bValue);;
            if(nIdx > (f32StartOfTurnrightlight) && nIdx < (f32StartOfReverselight) ) transmitBoolMediaSample(pTurnRightSampleSource,bValue);
            if(nIdx > (f32StartOfReverselight) && nIdx < ui32MaxLoopCount) transmitBoolMediaSample(pReverseLightSampleSource,bValue);
            // wait to control the sendrate
            cSystem::Sleep(nSleep);    
        }
    }
    //###############################################

    //####################################################################################################

    // clean up
    
    // disconnect the input pins of the aktors filter with the sample sources
    __adtf_test_result(pAccInPin->Disconnect(pAccSampleSource));
    __adtf_test_result(pSteerAngleInPin->Disconnect(pSteerAngleSampleSource));
    __adtf_test_result(pHeadLightInPin->Disconnect(pHeadLightSampleSource));
    __adtf_test_result(pTurnLeftInPin->Disconnect(pTurnLeftSampleSource));
    __adtf_test_result(pTurnRightInPin->Disconnect(pTurnRightSampleSource));
    __adtf_test_result(pBrakeLightInPin->Disconnect(pBrakeLightSampleSource));
    __adtf_test_result(pReverseLightInPin->Disconnect(pReverseLightSampleSource));
    __adtf_test_result(pEmergencyInPin->Disconnect(pEmergencySampleSource));

    __adtf_test_result(pArdCommInPin->Disconnect(pArdAktOutPin));
    __adtf_test_result(pArdSensInPin->Disconnect(pArdCommOutPin));

    // Disconnect the output pins of the sensors filter with the sample sinks
    __adtf_test_result(pArdSensUSOutPin->UnregisterEventSink(pUSSampleSink));
    __adtf_test_result(pArdSensIROutPin->UnregisterEventSink(pIRSampleSink));
    __adtf_test_result(pArdSensAccOutPin->UnregisterEventSink(pSteerAngleSampleSink));
    __adtf_test_result(pArdSensUSOutPin->UnregisterEventSink(pAccSampleSink));
    __adtf_test_result(pArdSensGyroOutPin->UnregisterEventSink(pGyroSampleSink));
    __adtf_test_result(pArdSensRPMOutPin->UnregisterEventSink(pRPMSampleSink));
    __adtf_test_result(pArdSensVoltageOutPin->UnregisterEventSink(pVoltageSampleSink));

    // shutdown the filter
    SET_STATE_SHUTDOWN(pAktFilter);
    SET_STATE_SHUTDOWN(pCommFilter);
    SET_STATE_SHUTDOWN(pSensFilter);
    SET_STATE_SHUTDOWN(pWDFilter);

    //#############################################################################################################################################
    // Start of evaluation
    //#############################################################################################################################################

    // now check the received data
    // Check for 80 samples, that should be received following the send rate of the arduino. Different sample count for US 50 and Voltage 10 due to the lower send rate
    __adtf_test_ext(pUSSampleSink->m_ui32SampleCount >= 50, cString::Format("Sample count (%d) does not match (%d)", pUSSampleSink->m_ui32SampleCount, 50));
    __adtf_test_ext(pIRSampleSink->m_ui32SampleCount >= 80, cString::Format("Sample count (%d) does not match (%d)", pIRSampleSink->m_ui32SampleCount, 80));
    __adtf_test_ext(pSteerAngleSampleSink->m_ui32SampleCount >= 80, cString::Format("Sample count (%d) does not match (%d)", pSteerAngleSampleSink->m_ui32SampleCount, 80));
    __adtf_test_ext(pAccSampleSink->m_ui32SampleCount >= 80, cString::Format("Sample count (%d) does not match (%d)", pAccSampleSink->m_ui32SampleCount, 80));
    __adtf_test_ext(pGyroSampleSink->m_ui32SampleCount >= 80, cString::Format("Sample count (%d) does not match (%d)", pGyroSampleSink->m_ui32SampleCount, 80));
    __adtf_test_ext(pRPMSampleSink->m_ui32SampleCount >= 80, cString::Format("Sample count (%d) does not match (%d)", pRPMSampleSink->m_ui32SampleCount, 80));
    __adtf_test_ext(pVoltageSampleSink->m_ui32SampleCount >= 10, cString::Format("Sample count (%d) does not match (%d)", pVoltageSampleSink->m_ui32SampleCount, 10));
    
    // If wer are in the acceleration or the complete test check for the ramp
    if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId == 0xff)evaluateRPM(pRPMSampleSink, chFrameId);
    // If wer are in the steerangle or the complete test check for the ramp
    if(chFrameId==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff) evaluateSteerAngle(pSteerAngleSampleSink);
    
    //#############################################################################################################################################
    // End of evaluation
    //#############################################################################################################################################
  
}


// Definition of the Test cases

DEFINE_TEST(cTesterAADCAktorsCommSensors,
            TestAcceleration,
            "4.1",
            "TestAcceleration",
            "This test makes sure, that the 'accelerate' pin is present. After that some acceleration data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive acceleration data and convert them into arduino data.",
            "automatic")
{
    // accelerate pin with expected frameID
    return DoActuatorCommSensorTest(ID_ARD_ACT_ACCEL_SERVO, 40000);
}



DEFINE_TEST(cTesterAADCAktorsCommSensors,
            TestSteeringAngle,
            "4.2",
            "TestSteeringAngle",
            "This test makes sure, that the 'steerAngle' pin is present. After that some steering data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive steering data and convert them into arduino data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoActuatorCommSensorTest(ID_ARD_ACT_STEER_ANGLE, 50000);
}


DEFINE_TEST(cTesterAADCAktorsCommSensors,
            TestLights,
            "4.3",
            "TestLights",
            "This test makes sure, that the 'LightPins' pins are present. After that some Lightsignals data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive Light data and convert them into arduino data.",
            "automatic")
{
    // Specific light test so no parameters needed
    return DoActuatorCommSensorTest(ID_ARD_ACT_LIGHT_DATA, 40000);
}

DEFINE_TEST(cTesterAADCAktorsCommSensors,
            TestAllActors,
            "4.4",
            "TestAllActors",
            "..."\
            "is set to different steering angle values",
            "The communication filter sends allways data",
            "See above",
            "none",
            "The filter must be able to receive steering data while delivering sensor data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoActuatorCommSensorTest(0xff, 40000);
}



 DEFINE_TEST_INACTIVE(cTesterAADCAktorsCommSensors,
            TestEmergencyStop,
            "1.5",
            "TestEmergencyStop",
            "This test makes sure, that the 'Emergency' pin is present. After that some Emergency data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive Emergency data and convert them into arduino data.",
            "automatic")
{
    // Do the BoolTest with the Watchdog pin and test the received samples
    return DoActuatorCommSensorTest(0xfe, 20000);
}