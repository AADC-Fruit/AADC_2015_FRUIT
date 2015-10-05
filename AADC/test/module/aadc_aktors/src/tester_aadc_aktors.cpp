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
#include "tester_aadc_aktors.h"

IMPLEMENT_TESTER_CLASS(cTesterAADCAktors,
                "1",
                "AADC Arduino Aktors filter",
                "This test makes sure that the AADC Arduino Aktors filter works as expected",
                "");


/*
Setup for this test
*/
void cTesterAADCAktors::setUp()
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
void cTesterAADCAktors::tearDown()
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
        // loop over all mediasamples to unref
        for (tInt nIdx = 0; nIdx < m_vecMediaSamples.size(); nIdx++)
        {
            // after unref the mediasample will destroy itself
            m_vecMediaSamples[nIdx]->Unref();
        }
    }

    // members to count and hold the received media samples
    tUInt32 m_ui32SampleCount;
    std::vector<IMediaSample*> m_vecMediaSamples;

public:
    tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample)
    {
        if (nEventCode == IPinEventSink::PE_MediaSampleTransmitted)
        {
            // count the samples
            ++m_ui32SampleCount;
            // make a ref on sample to avoid self destruction 
            pMediaSample->Ref();
            // push sample into vector for later compare
            m_vecMediaSamples.push_back(pMediaSample);
        }

        RETURN_NOERROR;
    }

};

tTestResult DoActuatorTest(const cString & strInputPinName, const tUInt8 chFrameId)
{
    // initialize filter
    INIT_FILTER(pFilter, pFilterConfig, "adtf.aadc.aktors");

    // set filter to state ready
    SET_STATE_READY(pFilter);

    // get the input pin
    cObjectPtr<IPin> pAccPin;
    __adtf_test_result_ext(pFilter->FindPin(strInputPinName.GetPtr(), IPin::PD_Input, &pAccPin), "Unable to find input pin");

    // get the output pin
    cObjectPtr<IPin> pOutput;
    __adtf_test_result_ext(pFilter->FindPin("ArduinoCOM_output", IPin::PD_Output, &pOutput), "Unable to find pin ArduinoCOM_output");

    // register the sample sink to receive the data from the output pin
    cObjectPtr<cMediaSampleSink> pSampleSink = new cMediaSampleSink();
    __adtf_test_result(pOutput->RegisterEventSink(pSampleSink));

    // create a simple sample source
    cObjectPtr<cOutputPin> pSampleSource = new cOutputPin();
    __adtf_test_result(pSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tSignalValue")));

    // connect the accelerate pin with th sample source
    __adtf_test_result(pAccPin->Connect(pSampleSource));

    // set the filter state running
    SET_STATE_RUNNING(pFilter);

    // get the mediatype of the sample source
    cObjectPtr<IMediaType> pType;
    __adtf_test_result(pSampleSource->GetMediaType(&pType)); 

    // get the type description
    cObjectPtr<IMediaTypeDescription> pTypeDesc;
    __adtf_test_result(pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&pTypeDesc));

    // get the serializer from description to get the deserialized size
    cObjectPtr<IMediaSerializer> pSerializer;
    __adtf_test_result(pTypeDesc->GetMediaSampleSerializer(&pSerializer));
    tInt nSize = pSerializer->GetDeserializedSize();
    pSerializer = NULL;

    // define values outside data range
    const tFloat32 f32LowerRange = -1.0f;
    const tFloat32 f32UpperRange = 181.0f;

    // define the loop count
    const tUInt32 ui32MaxLoopCount = 100;

    // send some samples
    for (tUInt32 nIdx = 0; nIdx < ui32MaxLoopCount; nIdx++)
    {
        // create and allocate the sample
        cObjectPtr<IMediaSample> pSample;
        __adtf_test_result(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**)&pSample));
        __adtf_test_result(pSample->AllocBuffer(nSize));

        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        __adtf_test_result(pTypeDesc->WriteLock(pSample, &pCoder));

        // set the value
        tFloat32 f32Value = 0.0f;
        if (0 == nIdx)
        {
            // first value must test the lower border of data range
            f32Value = f32LowerRange;
        }
        else if (ui32MaxLoopCount - 1 == nIdx)
        {
            // last value must test the upper border of data range
            f32Value = f32UpperRange;
        }
        else
        {
            // the rest will be calculated by index, range and loop count
            f32Value = (f32UpperRange - f32LowerRange) / ui32MaxLoopCount * nIdx;
        }

        // use the coder to set the value
        __adtf_test_result(pCoder->Set("f32Value", &f32Value));
        // unlock the coder
        __adtf_test_result(pTypeDesc->Unlock(pCoder));
        // set the sample time (in this case the time doesn't matter and must not be the stream time)
        pSample->SetTime(nIdx);
        // transmit the media sample
        __adtf_test_result(pSampleSource->Transmit(pSample));
    }

    // clean up
    __adtf_test_result(pAccPin->Disconnect(pSampleSource));
    __adtf_test_result(pOutput->UnregisterEventSink(pSampleSink));


    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));
    
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

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder //Search Dummy
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescArduinoData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values 
        tUInt8 ui8SOF = 0;
        tUInt8 ui8Id = 0;
        tUInt32 ui32ArduinoTimestamp = 0;
        tUInt8 ui8DataLength = 0;
        tUInt8 achFrameData[25];
        cMemoryBlock::MemSet(achFrameData, 0x00, sizeof(achFrameData));

        // get values from coder
        pCoder->Get("ui8SOF", (tVoid*)&ui8SOF);	
        pCoder->Get("ui8ID", (tVoid*)&ui8Id);
        pCoder->Get("ui32ArduinoTimestamp", (tVoid*)&(ui32ArduinoTimestamp));

        pCoder->Get("ui8DataLength", (tVoid*)&ui8DataLength);
        pCoder->Get("ui8Data", (tVoid*)&(achFrameData)); 

        // unlock 
        m_pCoderDescArduinoData->Unlock(pCoder);

        // test the received data  // Search Dummy
        // SOF
        __adtf_test(ui8SOF == ID_ARD_SOF);
        // Frame ID
        __adtf_test(ui8Id == chFrameId);
        // timestamp is always 0 in this filter
        __adtf_test(ui32ArduinoTimestamp == 0);
        __adtf_test(ui8DataLength == 1);

        if (nIdx == 0)
        { 
            // lower range check (values smaller than 0 must be set to 0)
            __adtf_test(achFrameData[0] == 0);
        }
        else if (nIdx == ui32MaxLoopCount)
        { 
            // upper range check (values greater than 180 must be set to 180)
            __adtf_test(achFrameData[0] == 180);
        }
        else
        {   
            // the value must be the same (with rounding) as set while transmit
            tFloat32 f32Value =  ((f32UpperRange - f32LowerRange) * static_cast<tFloat32> (nIdx) / ui32MaxLoopCount);

            __adtf_test_ext(static_cast<tUInt8> (f32Value  + 0.5f) == achFrameData[0], cString::Format("Received data (%d) not as expected (%d)", achFrameData[0], static_cast<tUInt8> (f32Value  + 0.5f)));
        }
    }

    // shutdown the filter
    SET_STATE_SHUTDOWN(pFilter);
}



tTestResult DoActuatorBoolTest(const cString & strInputPinName, const tUInt8 chFrameId)
{
    // initialize filter
    INIT_FILTER(pFilter, pFilterConfig, "adtf.aadc.aktors");

    // set filter to state ready
    SET_STATE_READY(pFilter);

    // get the input pin
    cObjectPtr<IPin> pBoolPin;
    __adtf_test_result_ext(pFilter->FindPin(strInputPinName.GetPtr(), IPin::PD_Input, &pBoolPin), "Unable to find input pin");

    // get the output pin
    cObjectPtr<IPin> pOutput;
    __adtf_test_result_ext(pFilter->FindPin("ArduinoCOM_output", IPin::PD_Output, &pOutput), "Unable to find pin ArduinoCOM_output");

    // register the sample sink to receive the data from the output pin
    cObjectPtr<cMediaSampleSink> pSampleSink = new cMediaSampleSink();
    __adtf_test_result(pOutput->RegisterEventSink(pSampleSink));

    // create a simple sample source
    cObjectPtr<cOutputPin> pSampleSource = new cOutputPin();

    if(chFrameId != ID_ARD_ACT_WD_ENABLE)
    {
        __adtf_test_result(pSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tBoolSignalValue")));
    }
    else
    {
        // we need another typ of samplesource
        __adtf_test_result(pSampleSource->Create("sampleSource", new cMediaType(0, 0, 0, "tJuryEmergencyStop")));
    }

    // connect the accelerate pin with th sample source
    __adtf_test_result(pBoolPin->Connect(pSampleSource));

    // set the filter state running
    SET_STATE_RUNNING(pFilter);

    // get the mediatype of the sample source
    cObjectPtr<IMediaType> pType;
    __adtf_test_result(pSampleSource->GetMediaType(&pType)); 

    // get the type description
    cObjectPtr<IMediaTypeDescription> pTypeDesc;
    __adtf_test_result(pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&pTypeDesc));

    // get the serializer from description to get the deserialized size
    cObjectPtr<IMediaSerializer> pSerializer;
    __adtf_test_result(pTypeDesc->GetMediaSampleSerializer(&pSerializer));
    tInt nSize = pSerializer->GetDeserializedSize();
    pSerializer = NULL;

    tUInt32 ui32MaxLoopCount = 50;

    // send some samples
    for (tUInt32 nIdx = 0; nIdx < ui32MaxLoopCount; nIdx++)
    {
        // create and allocate the sample
        cObjectPtr<IMediaSample> pSample;
        __adtf_test_result(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**)&pSample));
        __adtf_test_result(pSample->AllocBuffer(nSize));

        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        __adtf_test_result(pTypeDesc->WriteLock(pSample, &pCoder));

        // set the value
        tBool bValue = tFalse;

        // send every the first and then every second time on signal
        if(!(nIdx % 2))bValue = tTrue;
 
        // use the coder to set the value
        if(chFrameId != ID_ARD_ACT_WD_ENABLE)
        {
            __adtf_test_result(pCoder->Set("bValue", &bValue));
        }
        else
        {
            __adtf_test_result(pCoder->Set("bEmergencyStop", (tVoid*)&bValue)); 
        }
        

        // unlock the coder
        __adtf_test_result(pTypeDesc->Unlock(pCoder));
        // set the sample time (in this case the time doesn't matter and must not be the stream time)
        pSample->SetTime(nIdx);
        // transmit the media sample
        __adtf_test_result(pSampleSource->Transmit(pSample));
    }

    // clean up
    __adtf_test_result(pBoolPin->Disconnect(pSampleSource));
    __adtf_test_result(pOutput->UnregisterEventSink(pSampleSink));


    // now check the received data
    // first check for the right number of samples
    if (strInputPinName == "headLightEnabled")
    {
        // by sending headLight data the aktors filter automatically sends head and backlight data so the received samples have to be twice as much as the transmitted
        __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount * 2, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount * 2));
    }
    else if(strInputPinName == "Watchdog_Alive_Flag")
    {
        // only enabled watchdog signals with the value true shall be send to the arduino, so only have of the send samples should be received
        __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount / 2, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount / 2));
    }
    else if(chFrameId == ID_ARD_ACT_WD_ENABLE)
    {
        // only if the emergency stop value is true, the filter will send samples BUT the filter transmits a frame for the relais and a frame for watchdog
        // because we alternating the value this ends up in the same sample count
        __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount / 2));
    }
    else
    {
        // every send sample should generate one received sample
        __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));
    }
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

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder //Search Dummy
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescArduinoData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values 
        tUInt8 ui8SOF = 0;
        tUInt8 ui8Id = 0;
        tUInt32 ui32ArduinoTimestamp = 0;
        tUInt8 ui8DataLength = 0;
        tUInt8 aui8FrameData[25];
        cMemoryBlock::MemSet(aui8FrameData, 0x00, sizeof(aui8FrameData));

        // get values from coder
        pCoder->Get("ui8SOF", (tVoid*)&ui8SOF);	
        pCoder->Get("ui8ID", (tVoid*)&ui8Id);
        pCoder->Get("ui32ArduinoTimestamp", (tVoid*)&(ui32ArduinoTimestamp));

        pCoder->Get("ui8DataLength", (tVoid*)&ui8DataLength);
        pCoder->Get("ui8Data", (tVoid*)&(aui8FrameData)); 

        // unlock 
        m_pCoderDescArduinoData->Unlock(pCoder);

        // test the received data  // Search Dummy
        // SOF
        __adtf_test(ui8SOF == ID_ARD_SOF);
        // Frame ID only in the emergency test there will be samples send with different IDs
        if(chFrameId != ID_ARD_ACT_WD_ENABLE)
        {
            __adtf_test(ui8Id == chFrameId);
        }
        // timestamp is always 0 in this filter
        __adtf_test(ui32ArduinoTimestamp == 0);
        // datalength has to be 2 because of the light ID for watchdog only 1
        if(chFrameId == ID_ARD_ACT_WD_TOGGLE || chFrameId == ID_ARD_ACT_WD_ENABLE)
        {
            __adtf_test(ui8DataLength == 1);
        }
        else
        {
            __adtf_test(ui8DataLength == 2);
        }

        // Test the data of the protocol
        if (strInputPinName == "headLightEnabled")
        { 
            // the aktors filter sends two samples for head and back light so they have to be checked seperatedly
            if(nIdx == 0 || !(nIdx % 4))
            {
                // the first sample sends on to the head light
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_HEAD);
                __adtf_test(aui8FrameData[1] == tUInt8(tTrue));
            }
            else if(nIdx == 1 || !((nIdx-1) % 4))
            {
                // the second sample sends on to the back light
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_BACK);
                __adtf_test(aui8FrameData[1] == tUInt8(tTrue));
            }
            else if(nIdx == 2 || !((nIdx-2) % 4))
            {
                // the third sample sends off to the front light
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_HEAD);
                __adtf_test(aui8FrameData[1] == tUInt8(tFalse));
            }
            else if(nIdx == 3 || !((nIdx-3) % 4))
            {
                // the fourth sample sends off to the back light
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_BACK);
                __adtf_test(aui8FrameData[1] == tUInt8(tFalse));
            }
        }
        else if (strInputPinName == "brakeLightEnabled")
        { 
            // First on command then off command for brake light
            if(!(nIdx % 2))
            {
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_BRAKE);
                __adtf_test(aui8FrameData[1] == tUInt8(tTrue));
            }
            if(nIdx % 2)
            {
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_BRAKE);
                __adtf_test(aui8FrameData[1] == tUInt8(tFalse));
            }
        }
        else if (strInputPinName == "turnSignalLeftEnabled")
        { 
            // First on command then off command for turn left signal
            if(!(nIdx % 2))
            {
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_TURNLEFT);
                __adtf_test(aui8FrameData[1] == tUInt8(tTrue));
            }
            if(nIdx % 2)
            {
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_TURNLEFT);
                __adtf_test(aui8FrameData[1] == tUInt8(tFalse));
            }
        }
        else if (strInputPinName == "turnSignalRightEnabled")
        { 
            // First on command then off command for turn right signal
            if(!(nIdx % 2))
            {
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_TURNRIGHT);
                __adtf_test(aui8FrameData[1] == tUInt8(tTrue));
            }
            if(nIdx % 2)
            {
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_TURNRIGHT);
                __adtf_test(aui8FrameData[1] == tUInt8(tFalse));
            }
        }
        else if (strInputPinName == "reverseLightsEnabled")
        { 
            // First on command then off command for reverse light
            if(!(nIdx % 2))
            {
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_REVERSE);
                __adtf_test(aui8FrameData[1] == tUInt8(tTrue));
            }
            if(nIdx % 2)
            {
                __adtf_test(aui8FrameData[0] == ID_ARD_ACT_LIGHT_DATA_REVERSE);
                __adtf_test(aui8FrameData[1] == tUInt8(tFalse));
            }
        }
        else if (strInputPinName == "Watchdog_Alive_Flag")
        { 
            // Only true commands are send as watchdog signal
            __adtf_test(aui8FrameData[0] == tUInt8(tTrue));
        }
        else if (strInputPinName == "Emergency_Stop")
        { 
            // First Motor relais will be reseted then the watchdog will be killed
            if(!(nIdx % 2))
            {
                __adtf_test(aui8FrameData[0] == 0x00);
                __adtf_test(ui8Id == ID_ARD_ACT_MOT_RELAIS);
            }
            if(nIdx % 2){
                __adtf_test(aui8FrameData[0] == 0x00);
                __adtf_test(ui8Id == ID_ARD_ACT_WD_ENABLE);
            }
        }
    }

    // shutdown the filter
    SET_STATE_SHUTDOWN(pFilter);
}


// Definition of the Test cases

DEFINE_TEST(cTesterAADCAktors,
            TestAcceleration,
            "1.1",
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
    return DoActuatorTest("accelerate", ID_ARD_ACT_ACCEL_SERVO);
}



DEFINE_TEST(cTesterAADCAktors,
            TestSteeringAngle,
            "1.2",
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
    return DoActuatorTest("steerAngle", ID_ARD_ACT_STEER_ANGLE);
}

// helper function for the light tests
tTestResult LightTests(){
    // Do the light test for every available Light and test the received samples
    DoActuatorBoolTest("headLightEnabled", ID_ARD_ACT_LIGHT_DATA);
    DoActuatorBoolTest("brakeLightEnabled", ID_ARD_ACT_LIGHT_DATA);
    DoActuatorBoolTest("turnSignalLeftEnabled", ID_ARD_ACT_LIGHT_DATA);
    DoActuatorBoolTest("turnSignalRightEnabled", ID_ARD_ACT_LIGHT_DATA);
    DoActuatorBoolTest("reverseLightsEnabled", ID_ARD_ACT_LIGHT_DATA);  
}

DEFINE_TEST(cTesterAADCAktors,
            TestLights,
            "1.3",
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
    return LightTests();
}

 DEFINE_TEST(cTesterAADCAktors,
            TestWatchdog,
            "1.4",
            "TestWatchdog",
            "This test makes sure, that the 'Watchdog' pin is present. After that some Watchdog data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive Watchdog data and convert them into arduino data.",
            "automatic")
{
    // Do the BoolTest with the Watchdog pin and test the received samples
    return DoActuatorBoolTest("Watchdog_Alive_Flag", ID_ARD_ACT_WD_TOGGLE);
}

 DEFINE_TEST(cTesterAADCAktors,
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
    return DoActuatorBoolTest("Emergency_Stop", ID_ARD_ACT_WD_ENABLE);
}
