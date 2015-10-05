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
#include "tester_aadc_arduino_comm.h"

IMPLEMENT_TESTER_CLASS(cTesterAADCArduinoComm,
                "2",
                "AADC Arduino Communication filter",
                "This test makes sure that the AADC Arduino Communication filter works as expected",
                "");


/*
Setup for this test
*/
void cTesterAADCArduinoComm::setUp()
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
    FILTER_ENV_ADD_PLUGIN("aadc_arduinoCom.plb");

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
void cTesterAADCArduinoComm::tearDown()
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
/* This are just helper functions to send Media Samples.                                                                     */
/************************************************************************/

tTestResult transmitMediaSample(cOutputPin *pSampleSource, tUInt8 ui8Id, tUInt8 ui8DataLength, const tUInt8 *ui8Data)
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

    // init the values of the protocol
    tUInt8 ui8SOF = ID_ARD_SOF;         // Set the start of frame
    tUInt32 ui32ArduinoTimestamp = 0;       // Set the Timestamp     

    // create and allocate the sample
    cObjectPtr<IMediaSample> pSample;
    _runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**)&pSample);
    pSample->AllocBuffer(nSize);

    // get the coder
    cObjectPtr<IMediaCoder> pCoder;
    pTypeDesc->WriteLock(pSample, &pCoder);

    // use the coder to set the value
    pCoder->Set("ui8SOF", (tVoid*)&ui8SOF);    
    pCoder->Set("ui8ID", (tVoid*)&ui8Id);
    pCoder->Set("ui32ArduinoTimestamp", (tVoid*)&(ui32ArduinoTimestamp));
    pCoder->Set("ui8DataLength", (tVoid*)&ui8DataLength);
    pCoder->Set("ui8Data", (tVoid*)ui8Data);
 

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
tTestResult transmitDriveCommand(cOutputPin *pSampleSource, tFloat32 f32Value, const tUInt8 chFrameId)
{
    // correct rounding of the value  // Kommentar
    tUInt8 ui8Data = static_cast<tUInt8> (f32Value + 0.5f);
    // sizeof
    return transmitMediaSample(pSampleSource, chFrameId, 1, &ui8Data);
}
tTestResult transmitLightSignal(cOutputPin *pSampleSource, tUInt8 *ui8Data, const tUInt8 chFrameId)
{
    return transmitMediaSample(pSampleSource, chFrameId, 2, ui8Data);
}
tTestResult transmitWatchdog(cOutputPin *pSampleSource)
{
    tUInt8 ui8Data = static_cast<tUInt8> (true);
    return transmitMediaSample(pSampleSource, ID_ARD_ACT_WD_TOGGLE, 1, &ui8Data);
}


tTestResult DoCommunicationFilterTest(const tUInt8 chFrameId, tInt nSleep = 0)
{
    // initialize filter
    INIT_FILTER(pFilter, pFilterConfig, "adtf.aadc.arduinoCOM");

#ifdef WIN32 // WIN32 is also defined on WIN64
    // set the COM-Port on the Windows machine (on ODROID we use the default setting from filter)
    __adtf_test_result(pFilterConfig->SetPropertyStr("COM Port", " \\\\.\\COM9"));
#endif

    // set filter to state ready
    SET_STATE_READY(pFilter);

    // get the input pin of the Arduino Communications Filter
    cObjectPtr<IPin> pComInputPin;
    __adtf_test_result_ext(pFilter->FindPin("COM_input", IPin::PD_Input, &pComInputPin), "Unable to find input pin COM_input on Arduino Communications Filter");

    // get the output pin of the Arduino Communications Filter
    cObjectPtr<IPin> pComOutputPin;
    __adtf_test_result_ext(pFilter->FindPin("COM_output", IPin::PD_Output, &pComOutputPin), "Unable to find pin COM_output on Arduino Communications Filter");

    // register the sample sink to receive the data from the output pin of the Arduino Communication Filter
    cObjectPtr<cMediaSampleSink> pSampleSink = new cMediaSampleSink();
    __adtf_test_result(pComOutputPin->RegisterEventSink(pSampleSink));

    // create a simple sample source
    cObjectPtr<cOutputPin> pSampleSource = new cOutputPin();
    __adtf_test_result(pSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tArduinoData")));

    // connect the Input pin of the communication filter with the sample source
    __adtf_test_result(pComInputPin->Connect(pSampleSource));

    // set the filter state running
    SET_STATE_RUNNING(pFilter);

    // init the loop count
    tUInt32 ui32MaxLoopCount = 0;
    // init the wait time
    tUInt32 ui32Waittime = 0;
    
    //#############################################################################################################################################
    //Testing
    //#############################################################################################################################################
    
    // define the Test duration and the Waittime after Watchdog Signal is on
    ui32MaxLoopCount = 10000;
    // Dafines the Time from when the Watchdog is active to when the first actuator sample is sent. This is because of the initialisation of the motor controler
    ui32Waittime = 10000;

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
    tFloat32 f32NumberOfLights = 6;
    // Loopcount when the lights are activated
    tUInt32 ui32NumberOfCycles = ui32MaxLoopCount/4;

    // calculate the Start of each Lighttest
    tFloat32 f32StartOfHeadlight = ui32NumberOfCycles / f32NumberOfLights * 0;
    tFloat32 f32StartOfBacklight = ui32NumberOfCycles / f32NumberOfLights * 1;
    tFloat32 f32StartOfBrakelight = ui32NumberOfCycles / f32NumberOfLights * 2;
    tFloat32 f32StartOfTurnleftlight = ui32NumberOfCycles / f32NumberOfLights * 3;
    tFloat32 f32StartOfTurnrightlight = ui32NumberOfCycles / f32NumberOfLights * 4;
    tFloat32 f32StartOfReverselight = ui32NumberOfCycles / f32NumberOfLights * 5;

    //################################################################################################
    // send some samples
    //################################################
    // to give the motor controller time for initialisation send for a short time only wd signal 
    for (tUInt32 nIdx = 0; nIdx < ui32Waittime; nIdx++)
    {   
        // Allways send Watchdog Signal
        transmitWatchdog(pSampleSource);
        cSystem::Sleep(nSleep);
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
        // Allways send Watchdog Signal
        transmitWatchdog(pSampleSource);
        cSystem::Sleep(nSleep);

        // Acceleration and steer angle test
        if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId ==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            
            if(chFrameId == 0xff)
            {
                transmitDriveCommand(pSampleSource, f32Value,ID_ARD_ACT_ACCEL_SERVO);
                cSystem::Sleep(nSleep);
                transmitDriveCommand(pSampleSource, f32Value,ID_ARD_ACT_STEER_ANGLE);
                cSystem::Sleep(nSleep);
            }
            else
            {
                transmitDriveCommand(pSampleSource, f32Value,chFrameId);
                cSystem::Sleep(nSleep);
            }                    
        }
        // Light Test
        if(chFrameId == ID_ARD_ACT_LIGHT_DATA || chFrameId == 0xff)
        {
            // set the data of the protocol
            tBool bValue = tTrue;
            tUInt8 aui8Data[2] = {0,0};

            // Divide the loop into equal sections for every Light
            if(nIdx > (f32StartOfHeadlight) && nIdx < (f32StartOfBacklight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_HEAD;
            if(nIdx > (f32StartOfBacklight) && nIdx < (f32StartOfBrakelight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_BACK;
            if(nIdx > (f32StartOfBrakelight) && nIdx < (f32StartOfTurnleftlight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_BRAKE;
            if(nIdx > (f32StartOfTurnleftlight) && nIdx < (f32StartOfTurnrightlight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_TURNLEFT;
            if(nIdx > (f32StartOfTurnrightlight) && nIdx < (f32StartOfReverselight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_TURNRIGHT;
            if(nIdx > (f32StartOfReverselight) && nIdx < ui32MaxLoopCount) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_REVERSE;
            
            // Set the Lights to on
            bValue == tTrue ? aui8Data[1] = 1: aui8Data[1] = 0;

            transmitLightSignal(pSampleSource, aui8Data, ID_ARD_ACT_LIGHT_DATA);
            cSystem::Sleep(nSleep);        
        }
    }
    //###############################################
    // Go the same ramp backwards
    //####################
    // send the samples according to the specified ramp
    for (tUInt32 nIdx = ui32MaxLoopCount/4; nIdx > 0 ; nIdx--)
    {   
        // Allways send Watchdog Signal
        transmitWatchdog(pSampleSource);
        cSystem::Sleep(nSleep);

        // Acceleration and steer angle test
        if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId ==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            
            if(chFrameId == 0xff)
            {
                transmitDriveCommand(pSampleSource, f32Value,ID_ARD_ACT_ACCEL_SERVO);
                cSystem::Sleep(nSleep);
                transmitDriveCommand(pSampleSource, f32Value,ID_ARD_ACT_STEER_ANGLE);
                cSystem::Sleep(nSleep);
            }
            else
            {
                transmitDriveCommand(pSampleSource, f32Value,chFrameId);
                cSystem::Sleep(nSleep);
            }                    
        }
        // Light Test
        if(chFrameId == ID_ARD_ACT_LIGHT_DATA || chFrameId == 0xff)
        {
            // set the data of the protocol
            tBool bValue = tFalse;
            tUInt8 aui8Data[2] = {0,0};

            // Divide the loop into equal sections for every Light
            if(nIdx > (f32StartOfHeadlight) && nIdx < (f32StartOfBacklight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_HEAD;
            if(nIdx > (f32StartOfBacklight) && nIdx < (f32StartOfBrakelight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_BACK;
            if(nIdx > (f32StartOfBrakelight) && nIdx < (f32StartOfTurnleftlight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_BRAKE;
            if(nIdx > (f32StartOfTurnleftlight) && nIdx < (f32StartOfTurnrightlight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_TURNLEFT;
            if(nIdx > (f32StartOfTurnrightlight) && nIdx < (f32StartOfReverselight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_TURNRIGHT;
            if(nIdx > (f32StartOfReverselight) && nIdx < ui32MaxLoopCount) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_REVERSE;
            
            // Set the Lights to on
            bValue == tTrue ? aui8Data[1] = 1: aui8Data[1] = 0;

            transmitLightSignal(pSampleSource, aui8Data, ID_ARD_ACT_LIGHT_DATA);
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
        // Allways send Watchdog Signal
        transmitWatchdog(pSampleSource);
        cSystem::Sleep(nSleep);

        // Acceleration and steer angle test
        if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId ==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            
            if(chFrameId == 0xff)
            {
                transmitDriveCommand(pSampleSource, f32Value,ID_ARD_ACT_ACCEL_SERVO);
                cSystem::Sleep(nSleep);
                transmitDriveCommand(pSampleSource, f32Value,ID_ARD_ACT_STEER_ANGLE);
                cSystem::Sleep(nSleep);
            }
            else
            {
                transmitDriveCommand(pSampleSource, f32Value,chFrameId);
                cSystem::Sleep(nSleep);
            }                    
        }
        // Light Test
        if(chFrameId == ID_ARD_ACT_LIGHT_DATA || chFrameId == 0xff)
        {
            // set the data of the protocol
            tBool bValue = tTrue;
            tUInt8 aui8Data[2] = {0,0};

            // Divide the loop into equal sections for every Light
            if(nIdx > (f32StartOfHeadlight) && nIdx < (f32StartOfBacklight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_HEAD;
            if(nIdx > (f32StartOfBacklight) && nIdx < (f32StartOfBrakelight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_BACK;
            if(nIdx > (f32StartOfBrakelight) && nIdx < (f32StartOfTurnleftlight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_BRAKE;
            if(nIdx > (f32StartOfTurnleftlight) && nIdx < (f32StartOfTurnrightlight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_TURNLEFT;
            if(nIdx > (f32StartOfTurnrightlight) && nIdx < (f32StartOfReverselight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_TURNRIGHT;
            if(nIdx > (f32StartOfReverselight) && nIdx < ui32MaxLoopCount) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_REVERSE;
            
            // Set the Lights to on
            bValue == tTrue ? aui8Data[1] = 1: aui8Data[1] = 0;

            transmitLightSignal(pSampleSource, aui8Data, ID_ARD_ACT_LIGHT_DATA);
            cSystem::Sleep(nSleep);
        }
    }
    
    //###############################################
    // Go the same ramp backwards
    //####################
    // send the samples according to the specified ramp
    for (tUInt32 nIdx = 0; nIdx < ui32MaxLoopCount/4; nIdx++)
    {   
        // Allways send Watchdog Signal
        transmitWatchdog(pSampleSource);
        cSystem::Sleep(nSleep);

        // Acceleration and steer angle test
        if(chFrameId==ID_ARD_ACT_ACCEL_SERVO || chFrameId ==ID_ARD_ACT_STEER_ANGLE || chFrameId == 0xff)
        {
            // set the data of the protocol
            // Calculate the value by dividing the range by the loop count times the loop counter
            // through this every Value will be tested
            tFloat32 f32Value = (f32UpperRange - f32LowerRange) / (ui32MaxLoopCount/4) * nIdx + f32LowerRange;
            
            if(chFrameId == 0xff)
            {
                transmitDriveCommand(pSampleSource, f32Value,ID_ARD_ACT_ACCEL_SERVO);
                cSystem::Sleep(nSleep);
                transmitDriveCommand(pSampleSource, f32Value,ID_ARD_ACT_STEER_ANGLE);
                cSystem::Sleep(nSleep);
            }
            else
            {
                transmitDriveCommand(pSampleSource, f32Value,chFrameId);
                cSystem::Sleep(nSleep);
            }
        }
        // Light Test
        if(chFrameId == ID_ARD_ACT_LIGHT_DATA || chFrameId == 0xff)
        {
            // set the data of the protocol
            tBool bValue = tFalse;
            tUInt8 aui8Data[2] = {0,0};

            // Divide the loop into equal sections for every Light
            if(nIdx > (f32StartOfHeadlight) && nIdx < (f32StartOfBacklight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_HEAD;
            if(nIdx > (f32StartOfBacklight) && nIdx < (f32StartOfBrakelight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_BACK;
            if(nIdx > (f32StartOfBrakelight) && nIdx < (f32StartOfTurnleftlight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_BRAKE;
            if(nIdx > (f32StartOfTurnleftlight) && nIdx < (f32StartOfTurnrightlight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_TURNLEFT;
            if(nIdx > (f32StartOfTurnrightlight) && nIdx < (f32StartOfReverselight) ) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_TURNRIGHT;
            if(nIdx > (f32StartOfReverselight) && nIdx < ui32MaxLoopCount) aui8Data[0] = ID_ARD_ACT_LIGHT_DATA_REVERSE;
            
            // Set the Lights to on
            bValue == tTrue ? aui8Data[1] = 1: aui8Data[1] = 0;

            transmitLightSignal(pSampleSource, aui8Data, ID_ARD_ACT_LIGHT_DATA);
            cSystem::Sleep(nSleep);    
        }
    }
    //###############################################

    //####################################################################################################

    // clean up
    __adtf_test_result(pComInputPin->Disconnect(pSampleSource));
    __adtf_test_result(pComOutputPin->UnregisterEventSink(pSampleSink));
    
    //#############################################################################################################################################
    // Start of evaluation
    //#############################################################################################################################################

    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescArduino = pDescManager->GetMediaDescription("tArduinoData");
    __adtf_test_pointer(strDescArduino);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeArduinoData = new cMediaType(0, 0, 0, "tArduinoData", strDescArduino,IMediaDescription::MDF_DDL_DEFAULT_VERSION);    

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescArduinoData;
    __adtf_test_result(pTypeArduinoData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescArduinoData));

    // map with channel/frameid as key and vector of mediasamples as value to sort the different kinds of sensors
    //cSampleMap mapSensors;
    std::map<tChar, std::vector<tArduinoFrame> > mapSensors;

    // build the map by iterating over all received mediasamples
    for(tUInt32 nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescArduinoData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
            
        // init the values
        tUInt8 ui8SOF = 0;
        tUInt8 ui8Id = 0;
        tUInt32 arduinoTimestamp = 0;
	    tUInt8 ui8DataLength = 0;  
        tUInt8 frameData[25];

        // get values from coder    
        pCoder->Get("ui8ID", (tVoid*)&ui8Id);
        pCoder->Get("ui8SOF",  (tVoid*)&ui8SOF);
        pCoder->Get("ui32ArduinoTimestamp", (tVoid*)&arduinoTimestamp);
        pCoder->Get("ui8DataLength", (tVoid*)&ui8DataLength);
        pCoder->Get("ui8Data", (tVoid*)&(frameData));

         m_pCoderDescArduinoData->Unlock(pCoder);

        // check for plausible datalength
        ui8DataLength = ui8DataLength > sizeof(tArduinoDataUnion) ? sizeof(tArduinoDataUnion) : ui8DataLength;

        // create a arduinoframe
        tArduinoFrame sFrame;
        cMemoryBlock::MemSet(&sFrame,0, sizeof(tArduinoFrame));

        // fill the arduino frame
        sFrame.sHeader.ui8ID = ui8Id;
        sFrame.sHeader.ui8SOF = ui8SOF;
        sFrame.sHeader.ui32ArduinoTimestamp = arduinoTimestamp;
        sFrame.sHeader.ui8DataLength = ui8DataLength;
        cMemoryBlock::MemCopy(&sFrame.sData, &frameData, ui8DataLength);

        mapSensors[ui8Id].push_back(sFrame);
    }

    // clear the sample sink
    pSampleSink->m_vecMediaSamples.clear();
    // Shutdown the Filter
    SET_STATE_SHUTDOWN(pFilter);

    tUInt32 ui32test=mapSensors[ID_ARD_SENS_STEER_ANGLE].size();
    // Do a quick test to see if enough samples are received
    __adtf_test_ext(mapSensors[ID_ARD_SENS_STEER_ANGLE].size() >= 100, cString::Format("does not receive enough samples for Steerangle Sensor. Number of samples: %d", mapSensors[ID_ARD_SENS_STEER_ANGLE].size()));
    __adtf_test_ext(mapSensors[ID_ARD_SENS_WHEELENC].size() >= 100, cString::Format("does not receive enough samples for RPM Sensor. Number of samples: %d", mapSensors[ID_ARD_SENS_WHEELENC].size()));
    __adtf_test_ext(mapSensors[ID_ARD_SENS_IMU].size() >= 100, cString::Format("does not receive enough samples for Motion Sensor. Number of samples: %d", mapSensors[ID_ARD_SENS_IMU].size()));
    __adtf_test_ext(mapSensors[ID_ARD_SENS_IR].size() >= 100, cString::Format("does not receive enough samples for Infrared Sensor. Number of samples: %d", mapSensors[ID_ARD_SENS_IR].size()));
    __adtf_test_ext(mapSensors[ID_ARD_SENS_PHOTO].size() >= 100, cString::Format("does not receive enough samples for Photo Sensor. Number of samples: %d", mapSensors[ID_ARD_SENS_PHOTO].size()));
    __adtf_test_ext(mapSensors[ID_ARD_SENS_US].size() >= 50, cString::Format("does not receive enough samples for Ultra Sonic Sensor. Number of samples: %d", mapSensors[ID_ARD_SENS_US].size()));
    __adtf_test_ext(mapSensors[ID_ARD_SENS_VOLTAGE].size() >= 5, cString::Format("does not receive enough samples for Voltage Sensor. Number of samples: %d", mapSensors[ID_ARD_SENS_VOLTAGE].size()));

    // if the sleep time is long enough do a deeper check for the ramps
    if(nSleep > 10 && (chFrameId == ID_ARD_SENS_WHEELENC || chFrameId == 0xff))
    {
        // received sensor data should show the actor ramp
        // now check the received data

        // Define the Ranges of the ramp and a tolerance Value to ignore single value jumps
        tUInt16 ui16LowerRampRange = 1; 
        tUInt16 ui16UpperRampRange = 10; 
        tUInt16 ui16ToleranceValue = 4;

        // Init some values we need to recognize the ramp
        tUInt32 ui32RPMDiffBefore = 0;
        tUInt32 ui32RPMBefore = 0;
        tBool bStartOfRamp = tFalse;
        tInt8 i8RampSegment = 0;
        tUInt16 ui16StartRPMDiff = 0;
        
        // loop over the received data
        // rpm evaluation
        for(tSize nIdx = 0; nIdx < mapSensors[ID_ARD_SENS_WHEELENC].size(); ++nIdx)
        {
            // only take every third value to receive stronger differences
            if(nIdx % 3)
            {
                // test the received data
                // SOF
                __adtf_test(mapSensors[ID_ARD_SENS_WHEELENC][nIdx].sHeader.ui8SOF == ID_ARD_SOF);
                // Frame ID
                __adtf_test(mapSensors[ID_ARD_SENS_WHEELENC][nIdx].sHeader.ui8ID == ID_ARD_SENS_WHEELENC);
                // Check for the right datalength
                __adtf_test(mapSensors[ID_ARD_SENS_WHEELENC][nIdx].sHeader.ui8DataLength == 8);

                // Extract the Steerangle Data
                tUInt32 ui32RPM = mapSensors[ID_ARD_SENS_WHEELENC][nIdx].sData.sWheelEncData.ui32LeftWheel;
                // weigth the new value statistically with the old one to filter out peak values
                ui32RPM = (ui32RPMBefore + ui32RPM)/2;

                __adtf_test_log(cString::Format("RPM: %d", ui32RPM - ui32RPMBefore));
                __adtf_test_log(cString::Format("Rampsegment: %d", i8RampSegment));

                // Test for the ramp
                // recognize the start of the ramp
                if(!bStartOfRamp && ui32RPM - ui32RPMBefore > 0 && i8RampSegment == 0)
                {
                    ui16StartRPMDiff = 0;
                    bStartOfRamp = tTrue;
                    i8RampSegment++;
                }
                // recognize the first turn
                if(bStartOfRamp && ui32RPM - ui32RPMBefore > ui16UpperRampRange && i8RampSegment == 1)
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
                if(bStartOfRamp && ui32RPM - ui32RPMBefore == ui16StartRPMDiff && i8RampSegment == 3)
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
    //#############################################################################
    //Steerangle evaluation
    if(nSleep > 10 && (chFrameId == ID_ARD_SENS_STEER_ANGLE || chFrameId == 0xff))
    {
        // received sensor data should show the actor ramp
        // 0xff bei der Auswertung nicht vergessen
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
        for(tSize nIdx = 0; nIdx < mapSensors[ID_ARD_SENS_STEER_ANGLE].size(); ++nIdx)
        {
            // test the received data
            // SOF
            __adtf_test(mapSensors[ID_ARD_SENS_STEER_ANGLE][nIdx].sHeader.ui8SOF == ID_ARD_SOF);
            // Frame ID
            __adtf_test(mapSensors[ID_ARD_SENS_STEER_ANGLE][nIdx].sHeader.ui8ID == ID_ARD_SENS_STEER_ANGLE);
            // Check for the right datalength
            __adtf_test(mapSensors[ID_ARD_SENS_STEER_ANGLE][nIdx].sHeader.ui8DataLength == 2);

            // Extract the Steerangle Data
            tUInt16 ui16Angle = mapSensors[ID_ARD_SENS_STEER_ANGLE][nIdx].sData.sSteeringData.ui16Angle;

            __adtf_test_log(cString::Format("Steerangle: %d", ui16Angle));
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
}




DEFINE_TEST(cTesterAADCArduinoComm,
            TestAcceleration,
            "2.1",
            "TestAcceleration",
            "This test makes sure, that the output of the communication filter works while the input"\
            "is set to different acceleration values",
            "The communication filter sends allways data",
            "See above",
            "none",
            "The filter must be able to receive acceleration data while delivering sensor data.",
            "automatic")
{
    // accelerate pin with expected frameID
    return DoCommunicationFilterTest(ID_ARD_ACT_ACCEL_SERVO, 20);
}




DEFINE_TEST(cTesterAADCArduinoComm,
            TestSteeringAngle,
            "2.2",
            "TestSteeringAngle",
            "This test makes sure, that the output of the communication filter works while the input"\
            "is set to different steering angle values",
            "The communication filter sends allways data",
            "See above",
            "none",
            "The filter must be able to receive steering data while delivering sensor data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoCommunicationFilterTest(ID_ARD_ACT_STEER_ANGLE, 20);
}





DEFINE_TEST(cTesterAADCArduinoComm,
            TestLights,
            "2.2",
            "TestLights",
            "This test makes sure, that the output of the communication filter works while the input"\
            "is set to different steering angle values",
            "The communication filter sends allways data",
            "See above",
            "none",
            "The filter must be able to receive steering data while delivering sensor data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoCommunicationFilterTest(ID_ARD_ACT_LIGHT_DATA, 10);
}


DEFINE_TEST(cTesterAADCArduinoComm,
            TestAllActors,
            "2.3",
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
    return DoCommunicationFilterTest(0xff, 20);
}
