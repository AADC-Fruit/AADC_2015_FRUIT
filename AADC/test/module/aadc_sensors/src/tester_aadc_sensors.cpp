/**
 *
 * AADC Arduino sensors filter tests
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
#include "tester_aadc_sensors.h"

IMPLEMENT_TESTER_CLASS(cTesterAADCSensors,
                "1",
                "AADC Arduino sensors filter",
                "This test makes sure that the AADC Arduino sensors filter works as expected",
                "");


/*
Setup for this test
*/
void cTesterAADCSensors::setUp()
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
    FILTER_ENV_ADD_PLUGIN("aadc_sensors.plb");

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
void cTesterAADCSensors::tearDown()
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
        //// loop over all mediasamples to unref
        //for (tInt nIdx = 0; nIdx < m_vecMediaSamples.size(); nIdx++)
        //{
        //    // after unref the mediasample will destroy itself
        //    m_vecMediaSamples[nIdx]->Unref();
        //}
    }

    // members to count and hold the received media samples
    tUInt32 m_ui32SampleCount;
    std::vector<cObjectPtr<IMediaSample> > m_vecMediaSamples;

public:
    tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample)
    {
        if (nEventCode == IPinEventSink::PE_MediaSampleTransmitted)
        {
            // count the samples
            ++m_ui32SampleCount;
            // make a ref on sample to avoid self destruction 
            //pMediaSample->Ref();
            // push sample into vector for later compare
            m_vecMediaSamples.push_back(pMediaSample);
        }

        RETURN_NOERROR;
    }

};

/************************************************************************/
/* This is just a helper function to send data with the right protocoll.                                                             */
/************************************************************************/
tTestResult transmitMediaSample(cOutputPin *pSampleSource, const tUInt8 ui8Id, tUInt8 ui8DataLength, const tUInt8 *pui8Data)
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
    pCoder->Set("ui8Data", (tVoid*)pui8Data);
 

    // unlock the coder
    pTypeDesc->Unlock(pCoder);
    // set the sample time (in this case the time doesn't matter and must not be the stream time)
    pSample->SetTime(0);//cHighResTimer::GetTime());
    // transmit the media sample
    //__adtf_test_result(pSampleSource->Transmit(pSample));
    if(IS_FAILED(pSampleSource->Transmit(pSample)))
    {
        __adtf_test_ext(tFalse, "unable to send media sample");
    }

}

/************************************************************************/
/* These are just a helper functions to evaluate the received data                                                                    */
/************************************************************************/

tTestResult evaluateSteerAngle(cMediaSampleSink *pSampleSink, tUInt32 ui32MaxLoopCount)
{
    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));

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

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values 
        tUInt16 ui16SteerAngle = 0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        if(IS_FAILED(pCoder->Get("ui16Angle", (tVoid*)&ui16SteerAngle)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui32ArduinoTimestamp", (tVoid*)&ui32ArduinoTimestamp)))
        {   
            __adtf_test(tFalse);
        }

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        // test the received data 
        // SteerAngle
        if(ui16SteerAngle != nIdx)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16SteerAngle,nIdx));
        }
        // timestamp is always 0
        if(ui32ArduinoTimestamp != 0)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be 0",ui32ArduinoTimestamp));
        }

    }
}
tTestResult evaluateRPM(cMediaSampleSink *pSampleSink, tUInt32 ui32MaxLoopCount)
{

    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));

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

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values 
        tUInt32 ui32RPMLeft = 0;
        tUInt32 ui32RPMRight = 0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        if(IS_FAILED(pCoder->Get("ui32LeftWheel", (tVoid*)&ui32RPMLeft)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui32RightWheel", (tVoid*)&ui32RPMRight)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui32ArduinoTimestamp", (tVoid*)&ui32ArduinoTimestamp)))
        {   
            __adtf_test(tFalse);
        }

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        // test the received data 
        // RPM
        // Test for the first samples for Left RPM data, then test the right RPM data
        if(nIdx < 65280)
        {
            if(ui32RPMLeft!=nIdx)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui32RPMLeft,nIdx));
            }
        }
        if(nIdx >= 65280) 
        {
            if(ui32RPMRight!=nIdx-65280)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui32RPMRight,nIdx-65280));
            }
        }
        // timestamp is always 0 
        if(ui32ArduinoTimestamp != 0)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be 0",ui32ArduinoTimestamp));
        }

    }
}
tTestResult evaluateGyro(cMediaSampleSink *pSampleSink, tUInt32 ui32MaxLoopCount)
{
    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));

    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescRPM = pDescManager->GetMediaDescription("tGyroData");
    __adtf_test_pointer(strDescRPM);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeSteerAngleData = new cMediaType(0, 0, 0, "tGyroData", strDescRPM,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSteerAngleData;
    __adtf_test_result(pTypeSteerAngleData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSteerAngleData)); 

    // define the maximum Value for each sensor
    const tUInt32 ui32MaxValue = 65280;

    // loop over the received data
    for(tSize nIdx = 0; nIdx < ui32MaxValue*4 && nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);

        // init the values 
        tInt16 i16Q_w = 0;
        tInt16 i16Q_x = 0;
        tInt16 i16Q_y = 0;
        tInt16 i16Q_z = 0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        if(IS_FAILED(pCoder->Get("i16Q_w", &i16Q_w)))
        {   
            __adtf_test(tFalse);
        }	
        if(IS_FAILED(pCoder->Get("i16Q_x", &i16Q_x)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("i16Q_y", &i16Q_y)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("i16Q_z", &i16Q_z)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui32ArduinoTimestamp", &ui32ArduinoTimestamp)))
        {   
            __adtf_test(tFalse);
        }

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        // test the received data 
        // Gyro
        // Test the Gyro data, as in the acc test there are also negative values.
        if(nIdx < ui32MaxValue*1)
        {
            if(i16Q_w>=0)
            {
                if(i16Q_w!=nIdx)
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16Q_w,nIdx));
                }
            }else{
                if(i16Q_w!=-65536+nIdx)
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16Q_w,-65536+nIdx));
                }
            }
        }else if(nIdx < ui32MaxValue*2)
        {
            if(i16Q_x>=0)
            {
                if(i16Q_x!=(nIdx-ui32MaxValue*1))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16Q_x,(nIdx-ui32MaxValue*1)));
                }
            }else{
                if(i16Q_x!=-65536+(nIdx-ui32MaxValue*1))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16Q_x,-65536+(nIdx-ui32MaxValue*1)));
                }
            }
        }else if(nIdx < ui32MaxValue*3)
        {
            if(i16Q_y>=0)
            {
                if(i16Q_y!=(nIdx-ui32MaxValue*2))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16Q_y,(nIdx-ui32MaxValue*2)));
                }
            }else{
                if(i16Q_y!=-65536+(nIdx-ui32MaxValue*2))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16Q_y,-65536+(nIdx-ui32MaxValue*2)));
                }
            }
        }else
        {
            if(i16Q_z>=0)
            {
                if(i16Q_z!=(nIdx-ui32MaxValue*3))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16Q_z,(nIdx-ui32MaxValue*3)));
                }
            }else{
                if(i16Q_z!=-65536+(nIdx-ui32MaxValue*3))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16Q_z,-65536+(nIdx-ui32MaxValue*3)));
                }
            }
        }
        // timestamp is always 0 
        if(ui32ArduinoTimestamp != 0)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (0)",ui32ArduinoTimestamp));
        }
    }
}
tTestResult evaluateAcc(cMediaSampleSink *pSampleSink, tUInt32 ui32MaxLoopCount)
{
    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));

    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescRPM = pDescManager->GetMediaDescription("tAccData");
    __adtf_test_pointer(strDescRPM);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeSteerAngleData = new cMediaType(0, 0, 0, "tAccData", strDescRPM,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSteerAngleData;
    __adtf_test_result(pTypeSteerAngleData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSteerAngleData)); 

    // define the maximum Value for each sensor
    const tUInt32 ui32MaxValue = 65280;

    // loop over the received data
    for(tSize nIdx = ui32MaxValue*4; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);

        // init the values 
        tInt16 i16A_x = 0;
        tInt16 i16A_y = 0;
        tInt16 i16A_z = 0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        if(IS_FAILED(pCoder->Get("i16A_x", &i16A_x)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("i16A_y", &i16A_y)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("i16A_z", &i16A_z)))
        {   
            __adtf_test(tFalse);
        }
        // useless because there are the same values as in i16A_x
        // pCoder->Get("i16Temperature", &i16A_x));
        if(IS_FAILED(pCoder->Get("ui32ArduinoTimestamp", &ui32ArduinoTimestamp)))
        {   
            __adtf_test(tFalse);
        }

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        // test the received data 
        // Acc
        // Test the three axes one after the other
        if(nIdx < ui32MaxValue*5)
        {
            // Because there are also negative values we have to test them too
            if(i16A_x>=0)
            {
                if(i16A_x!=(nIdx-ui32MaxValue*4))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16A_x,(nIdx-ui32MaxValue*4)));
                }
            }else{
                if(i16A_x!=-65536+(nIdx-ui32MaxValue*4))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16A_x,-65536+(nIdx-ui32MaxValue*4)));
                }
            }
        }else if(nIdx < ui32MaxValue*6)
        {
            if(i16A_y>=0)
            {
                if(i16A_y!=(nIdx-ui32MaxValue*5))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16A_y,(nIdx-ui32MaxValue*5)));
                }
            }else{
                if(i16A_y!=-65536+(nIdx-ui32MaxValue*5))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16A_y,-65536+(nIdx-ui32MaxValue*5)));
                }
            }
        }else if(nIdx < ui32MaxValue*7)
        {
            if(i16A_z>=0)
            {
                if(i16A_z!=(nIdx-ui32MaxValue*6))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16A_z,(nIdx-ui32MaxValue*6)));
                }
            }else{
                if(i16A_z!=-65536+(nIdx-ui32MaxValue*6))
                {
                    __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",i16A_z,-65536+(nIdx-ui32MaxValue*6)));
                }
            }
        }else{
            // Do nothing because there is the temperature data which is not usefull
        }
        // timestamp is always 0 
        if(ui32ArduinoTimestamp != 0)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (0)",ui32ArduinoTimestamp));
        }
    }
}
tTestResult evaluateIR(cMediaSampleSink *pSampleSink, tUInt32 ui32MaxLoopCount)
{    
    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));

    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescRPM = pDescManager->GetMediaDescription("tIrData");
    __adtf_test_pointer(strDescRPM);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeSteerAngleData = new cMediaType(0, 0, 0, "tIrData", strDescRPM,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSteerAngleData;
    __adtf_test_result(pTypeSteerAngleData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSteerAngleData)); 

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values
        tUInt16 ui16Front_Center_Longrange=0;
        tUInt16 ui16Front_Center_Shortrange=0;
        tUInt16 ui16Front_Left_Longrange=0;
        tUInt16 ui16Front_Left_Shortrange=0;
        tUInt16 ui16Front_Right_Shortrange=0;
        tUInt16 ui16Front_Right_Longrange=0;
        tUInt16 ui16Rear_Center_Shortrange=0;
        tUInt16 ui16Rear_Left_Shortrange=0;
        tUInt16 ui16Rear_Right_Shortrange=0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        if(IS_FAILED(pCoder->Get("ui16Front_Center_Longrange", &ui16Front_Center_Longrange)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui16Front_Center_Shortrange", &ui16Front_Center_Shortrange)))
        {   
            __adtf_test(tFalse);
        }    
        if(IS_FAILED(pCoder->Get("ui16Front_Left_Longrange", &ui16Front_Left_Longrange)))
        {   
            __adtf_test(tFalse);
        }    
        if(IS_FAILED(pCoder->Get("ui16Front_Left_Shortrange", &ui16Front_Left_Shortrange)))
        {   
            __adtf_test(tFalse);
        }    
        if(IS_FAILED(pCoder->Get("ui16Front_Right_Shortrange", &ui16Front_Right_Shortrange)))
        {   
            __adtf_test(tFalse);
        }    
        if(IS_FAILED(pCoder->Get("ui16Front_Right_Longrange", &ui16Front_Right_Longrange)))
        {   
            __adtf_test(tFalse);
        }    
        if(IS_FAILED(pCoder->Get("ui16Rear_Center_Shortrange", &ui16Rear_Center_Shortrange)))
        {   
            __adtf_test(tFalse);
        }    
        if(IS_FAILED(pCoder->Get("ui16Rear_Left_Shortrange", &ui16Rear_Left_Shortrange)))
        {   
            __adtf_test(tFalse);
        }    
        if(IS_FAILED(pCoder->Get("ui16Rear_Right_Shortrange", &ui16Rear_Right_Shortrange)))
        {   
            __adtf_test(tFalse);
        }  

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        tUInt32 ui32MaxValue = 65280;

        // test the received data 
        // IR
        // Test every single Sensor because of the send data they are counted up one after the other
        if(nIdx < ui32MaxValue*1)
        {
            if(ui16Front_Center_Longrange!=nIdx)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Front_Center_Longrange,nIdx));
            }
        }else if(nIdx < ui32MaxValue*2)
        {
            if(ui16Front_Center_Shortrange!=nIdx-ui32MaxValue*1)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Front_Center_Shortrange,nIdx));
            }
        }else if(nIdx < ui32MaxValue*3)
        {
            if(ui16Front_Left_Longrange!=nIdx-ui32MaxValue*2)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Front_Left_Longrange,nIdx-ui32MaxValue*2));
            }
        }else if(nIdx < ui32MaxValue*4)
        {
            if(ui16Front_Left_Shortrange!=nIdx-ui32MaxValue*3)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Front_Left_Shortrange,nIdx-ui32MaxValue*3));
            }
        }else if(nIdx < ui32MaxValue*5)
        {
            if(ui16Front_Right_Longrange!=nIdx-ui32MaxValue*4)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Front_Right_Longrange,nIdx-ui32MaxValue*5));
            }
        }else if(nIdx < ui32MaxValue*6)
        {
            if(ui16Front_Right_Shortrange!=nIdx-ui32MaxValue*5)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Front_Right_Shortrange,nIdx-ui32MaxValue*4));
            }
        }else if(nIdx < ui32MaxValue*7)
        {
            if(ui16Rear_Center_Shortrange!=nIdx-ui32MaxValue*6)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Rear_Center_Shortrange,nIdx-ui32MaxValue*6));
            }
        }else if(nIdx < ui32MaxValue*8)
        {
            if(ui16Rear_Left_Shortrange!=nIdx-ui32MaxValue*7)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Rear_Left_Shortrange,nIdx-ui32MaxValue*7));
            }
        }else if(nIdx < ui32MaxValue*9)
        {
            if(ui16Rear_Right_Shortrange!=nIdx-ui32MaxValue*8)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Rear_Right_Shortrange,nIdx-ui32MaxValue*8));
            }
        }
        // timestamp is always 0 
        if(ui32ArduinoTimestamp != 0)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (0)",ui32ArduinoTimestamp));
        }

    }
}
/**
tTestResult evaluatePHOTO(cMediaSampleSink *pSampleSink, tUInt32 ui32MaxLoopCount)
{
    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));

    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescSteerAngle = pDescManager->GetMediaDescription("tPhotoData");
    __adtf_test_pointer(strDescSteerAngle);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeSteerAngleData = new cMediaType(0, 0, 0, "tPhotoData", strDescSteerAngle,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSteerAngleData;
    __adtf_test_result(pTypeSteerAngleData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSteerAngleData)); 

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values 
        tUInt32 ui32luminosity = 0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        pCoder->Get("ui32luminosity", (tVoid*)&ui32luminosity);
        pCoder->Get("ui32ArduinoTimestamp", (tVoid*)&ui32ArduinoTimestamp);

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        // test the received data 
        // SteerAngle
        if(ui32luminosity != nIdx)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui32luminosity,nIdx));
        }
        // timestamp is always 0
        if(ui32ArduinoTimestamp != 0)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be 0",ui32ArduinoTimestamp));
        }

    }
}
**/
tTestResult evaluateUS(cMediaSampleSink *pSampleSink, tUInt32 ui32MaxLoopCount)
{    
    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));

    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescRPM = pDescManager->GetMediaDescription("tUsData");
    __adtf_test_pointer(strDescRPM);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeSteerAngleData = new cMediaType(0, 0, 0, "tUsData", strDescRPM,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSteerAngleData;
    __adtf_test_result(pTypeSteerAngleData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSteerAngleData)); 

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values
        tUInt16 ui16FrontLeft=0;
        tUInt16 ui16FrontRight=0;
        tUInt16 ui16RearLeft=0;
        tUInt16 ui16RearRight=0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        if(IS_FAILED(pCoder->Get("ui16Front_Left", &ui16FrontLeft)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui16Front_Right", &ui16FrontRight)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui16Rear_Left", &ui16RearLeft)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui16Rear_Right", &ui16RearRight)))
        {   
            __adtf_test(tFalse);
        }     
        if(IS_FAILED(pCoder->Get("ui32ArduinoTimestamp", &ui32ArduinoTimestamp)))
        {   
            __adtf_test(tFalse);
        }   

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        tUInt32 ui32MaxValue = 65280;

        // test the received data 
        // US
        // Test for the first samples for US Front Left data, 
        if(nIdx < ui32MaxValue*1)
        {
            if(ui16FrontLeft!=nIdx)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16FrontLeft,nIdx));
            }
        // then test the Front right data
        }else if(nIdx < ui32MaxValue*2)
        {
            if(ui16FrontRight!=nIdx-ui32MaxValue*1)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16FrontRight,nIdx));
            }
        // now the rear left data
        }else if(nIdx < ui32MaxValue*3)
        {
            if(ui16RearLeft!=nIdx-ui32MaxValue*2)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16RearLeft,nIdx-ui32MaxValue*2));
            }
        // and finally the rear right data
        }else if(nIdx < ui32MaxValue*4)
        {
            if(ui16RearRight!=nIdx-ui32MaxValue*3)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16RearRight,nIdx-ui32MaxValue*3));
            }
        }
        // timestamp is always 0 
        if(ui32ArduinoTimestamp != 0)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (0)",ui32ArduinoTimestamp));
        }

    }
}
tTestResult evaluateVOLTAGE(cMediaSampleSink *pSampleSink, tUInt32 ui32MaxLoopCount)

{    
    // now check the received data
    __adtf_test_ext(pSampleSink->m_ui32SampleCount == ui32MaxLoopCount, cString::Format("Sample count (%d) does not match (%d)", pSampleSink->m_ui32SampleCount, ui32MaxLoopCount));

    // get description manager
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    __adtf_test_result(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager));

    // get media description for tArduinoData
    tChar const * strDescRPM = pDescManager->GetMediaDescription("tVoltageData");
    __adtf_test_pointer(strDescRPM);

    // create mediatype for coder
    cObjectPtr<IMediaType> pTypeSteerAngleData = new cMediaType(0, 0, 0, "tVoltageData", strDescRPM,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	

    // get the mediatype description from the mediatype
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSteerAngleData;
    __adtf_test_result(pTypeSteerAngleData->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSteerAngleData)); 

    // loop over the received data
    for(tSize nIdx = 0; nIdx < pSampleSink->m_vecMediaSamples.size(); ++nIdx)
    {
        // get the coder
        cObjectPtr<IMediaCoder> pCoder;
        m_pCoderDescSteerAngleData->Lock(pSampleSink->m_vecMediaSamples[nIdx], &pCoder);
        
        // init the values
        tUInt16 ui16Measurement=0;
        tUInt16 ui16Power=0;
        tUInt32 ui32ArduinoTimestamp = 0;

        // get values from coder
        if(IS_FAILED(pCoder->Get("ui32MeasurementCircuit",&ui16Measurement)))
        {   
            __adtf_test(tFalse);
        }
        if(IS_FAILED(pCoder->Get("ui32PowerCircuit", &ui16Power)))
        {   
            __adtf_test(tFalse);
        }     
        if(IS_FAILED(pCoder->Get("ui32ArduinoTimestamp", &ui32ArduinoTimestamp)))
        {   
            __adtf_test(tFalse);
        }   

        // unlock 
        m_pCoderDescSteerAngleData->Unlock(pCoder);

        tUInt32 ui32MaxValue = 65280;

        // test the received data 
        // voltage
        // Test for the first samples for MeasurementCircuit data, then test the PowerCircuit data
        if(nIdx < ui32MaxValue*1)
        {
            if(ui16Measurement!=nIdx)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Measurement,nIdx));
            }
        }else
        {
            if(ui16Power!=nIdx-ui32MaxValue*1)
            {
                __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (%d)",ui16Power,nIdx));
            }
        }
        // timestamp is always 0 
        if(ui32ArduinoTimestamp != 0)
        {
            __adtf_test_ext(tFalse,cString::Format("Wrong Value is(%d) should be (0)",ui32ArduinoTimestamp));
        }

    }
}

tTestResult DoSensorTest(const cString & strInputPinName, const tUInt8 ui8FrameId)
{
    // initialize filter
    INIT_FILTER(pFilter, pFilterConfig, "adtf.aadc.sensors");

    // set filter to state ready
    SET_STATE_READY(pFilter);

    // get the input pin
    cObjectPtr<IPin> pCommPin;
    __adtf_test_result_ext(pFilter->FindPin("ArduinoCOM_input", IPin::PD_Input, &pCommPin), "Unable to find input pin");

    // get the output pin
    cObjectPtr<IPin> pOutput;
    __adtf_test_result_ext(pFilter->FindPin(strInputPinName.GetPtr(), IPin::PD_Output, &pOutput), cString::Format("Unable to find pin %s",strInputPinName.GetPtr()));

    // register the sample sink to receive the data from the output pin
    cObjectPtr<cMediaSampleSink> pSampleSink = new cMediaSampleSink();
    __adtf_test_result(pOutput->RegisterEventSink(pSampleSink))

    // for each imu sample the sensors filter sends a acc sample and a gyro sample, so we need two seperated sample sinks.
    // beacause of the test parameter gyroscope there is already a gyroscope sink registered, so now we need a accelerometer sink
    cObjectPtr<IPin> pOutputAcc;
    cObjectPtr<cMediaSampleSink> pSampleSinkAcc = new cMediaSampleSink();
    if(ui8FrameId == ID_ARD_SENS_IMU)
    {
        // get the output pin 
        __adtf_test_result_ext(pFilter->FindPin("accelerometer", IPin::PD_Output, &pOutputAcc), "Unable to find Accelerometer pin" );
        // register the sample sink to receive the data from the output pin
        __adtf_test_result(pOutputAcc->RegisterEventSink(pSampleSinkAcc));
    }


    ////#########################################################################################################
    //// Sample Sink on every output pin
    ////#########################################################################################################

    //// get the output pin
    //cObjectPtr<IPin> pOutput2;
    //__adtf_test_result_ext(pFilter->FindPin("gyroscope", IPin::PD_Output, &pOutput2), cString::Format("Unable to find pin %s",strInputPinName));

    //// register the sample sink to receive the data from the output pin
    //cObjectPtr<cMediaSampleSink> pSampleSink2 = new cMediaSampleSink();
    //__adtf_test_result(pOutput2->RegisterEventSink(pSampleSink2));

    //// get the output pin
    //cObjectPtr<IPin> pOutput3;
    //__adtf_test_result_ext(pFilter->FindPin("infrared_sensors", IPin::PD_Output, &pOutput3), cString::Format("Unable to find pin %s",strInputPinName));

    //// register the sample sink to receive the data from the output pin
    //cObjectPtr<cMediaSampleSink> pSampleSink3 = new cMediaSampleSink();
    //__adtf_test_result(pOutput3->RegisterEventSink(pSampleSink3));

    //// get the output pin
    //cObjectPtr<IPin> pOutput4;
    //__adtf_test_result_ext(pFilter->FindPin("ultrasonic_sensors", IPin::PD_Output, &pOutput4), cString::Format("Unable to find pin %s",strInputPinName));

    //// register the sample sink to receive the data from the output pin
    //cObjectPtr<cMediaSampleSink> pSampleSink4 = new cMediaSampleSink();
    //__adtf_test_result(pOutput4->RegisterEventSink(pSampleSink4));

    //// get the output pin
    //cObjectPtr<IPin> pOutput5;
    //__adtf_test_result_ext(pFilter->FindPin("system_voltage", IPin::PD_Output, &pOutput5), cString::Format("Unable to find pin %s",strInputPinName));

    //// register the sample sink to receive the data from the output pin
    //cObjectPtr<cMediaSampleSink> pSampleSink5 = new cMediaSampleSink();
    //__adtf_test_result(pOutput5->RegisterEventSink(pSampleSink5));
    ////#########################################################################################################
    //// Sample Sink on every output pin
    ////#########################################################################################################

    // create a simple sample source
    cObjectPtr<cOutputPin> pSampleSource = new cOutputPin();
    __adtf_test_result(pSampleSource->Create("sampleSource", new cMediaType(0,0,0,"tArduinoData")));

    // connect the communication pin with the sample source
    __adtf_test_result(pCommPin->Connect(pSampleSource));

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

    

    // init the data of the protocol
    tUInt8 aui8Data[25]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    // define the loop count, also defines the datarange
    tUInt32 ui32MaxLoopCount = 20000;
    // define the position to write data
    tUInt8 ui8PositionToWrite = 0;
    // define the size of a single value
    tUInt8 ui8SizeOfValue = 2;
    // define the value to write
    tUInt16 ui16Value = 0;
    // init the Datalength of the protocol
    tUInt8 ui8DataLength = 0;
    // define the Datalength and the size of the values of the cuurent protocol 
    if(ui8FrameId == ID_ARD_SENS_STEER_ANGLE)
    {
        ui8DataLength = 2;
    }
    if(ui8FrameId == ID_ARD_SENS_WHEELENC)
    {
        ui8DataLength = 8;
        ui8SizeOfValue = 4;
    }
    if(ui8FrameId == ID_ARD_SENS_IMU)
    {
        ui8DataLength = 16;
    }
    if(ui8FrameId == ID_ARD_SENS_IR)
    {
        ui8DataLength = 18;
    }
    if(ui8FrameId == ID_ARD_SENS_PHOTO)
    {
        ui8DataLength = 2;
    }
    if(ui8FrameId == ID_ARD_SENS_US)
    {   
        ui8DataLength = 8;
    }
    if(ui8FrameId == ID_ARD_SENS_VOLTAGE)
    {
        ui8DataLength = 4;
    }

    // send some samples
    for (tUInt32 nIdx = 0; nIdx < ui32MaxLoopCount; nIdx++)
    {
        // Because of 8 bit in each byte we want to increase the next higher byte with each full lower byte
        if(aui8Data[ui8PositionToWrite] == static_cast <tUInt8> (255))
        {   
            aui8Data[ui8PositionToWrite+1] += 1;
            ui16Value = 0;
        }
        // If the current value range is full go to the next value range
        if(aui8Data[ui8PositionToWrite+1] == static_cast <tUInt8> (255))
        {
            ui16Value = 0;
            ui8PositionToWrite += ui8SizeOfValue;
        }
        // Check if the end of the current protocol is reached. if yes quit the loop but set the goal of received samples to match the send samples
        if(ui8PositionToWrite == ui8DataLength)
        {
            ui32MaxLoopCount = nIdx;
            break;
        }

        // set the actual value
        aui8Data[ui8PositionToWrite]= ui16Value;

        // transmitt the value
        transmitMediaSample(pSampleSource,ui8FrameId,ui8DataLength, aui8Data);
        // increase the Value for the next time.
        ui16Value++;

    }

    // clean up
    __adtf_test_result(pCommPin->Disconnect(pSampleSource));
    __adtf_test_result(pOutput->UnregisterEventSink(pSampleSink));

    //##############################################################
    // Evaluation
    //##############################################################

    if(ui8FrameId == ID_ARD_SENS_STEER_ANGLE)
    {
        evaluateSteerAngle(pSampleSink, ui32MaxLoopCount);
    }
    if(ui8FrameId == ID_ARD_SENS_WHEELENC)
    {
        evaluateRPM(pSampleSink, ui32MaxLoopCount);
    }
    if(ui8FrameId == ID_ARD_SENS_IMU)
    {
        evaluateGyro(pSampleSink, ui32MaxLoopCount);
        evaluateAcc(pSampleSinkAcc, ui32MaxLoopCount);
    }
    if(ui8FrameId == ID_ARD_SENS_IR)
    {
        evaluateIR(pSampleSink, ui32MaxLoopCount);
    }
    //if(ui8FrameId == ID_ARD_SENS_PHOTO)evaluatePHOTO(pSampleSink, ui32MaxLoopCount);
    if(ui8FrameId == ID_ARD_SENS_US)
    {
        evaluateUS(pSampleSink, ui32MaxLoopCount);
    }
    if(ui8FrameId == ID_ARD_SENS_VOLTAGE)
    {
        evaluateVOLTAGE(pSampleSink, ui32MaxLoopCount);
    }
    
    // shutdown the filter
    SET_STATE_SHUTDOWN(pFilter);
}


DEFINE_TEST(cTesterAADCSensors,
            TestSteeringAngle,
            "3.1",
            "TestSteeringAngle",
            "This test makes sure, that the 'SteeringAngle' pin is present. After that some SteeringAngle data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive Arduino data and convert them into SteeringAngle data.",
            "automatic")
{
    // accelerate pin with expected frameID
    return DoSensorTest("steering_servo", ID_ARD_SENS_STEER_ANGLE);
}





DEFINE_TEST(cTesterAADCSensors,
            TestRPM,
            "3.2",
            "TestRPM",
            "This test makes sure, that the 'RPM' pin is present. After that some RPM data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive arduino data and convert them into rpm data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoSensorTest("wheel_speed_sensor", ID_ARD_SENS_WHEELENC);
}





DEFINE_TEST(cTesterAADCSensors,
            TestIMU,
            "3.3",
            "TestIMU",
            "This test makes sure, that the 'Acc and Gyro' pins are present. After that some IMU data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive arduino data and convert them into acc and gyro data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoSensorTest("gyroscope", ID_ARD_SENS_IMU);
}





DEFINE_TEST(cTesterAADCSensors,
            TestIR,
            "3.4",
            "TestIR",
            "This test makes sure, that the 'IR' pin is present. After that some ir data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive arduino data and convert them into IR data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoSensorTest("infrared_sensors", ID_ARD_SENS_IR);
}





DEFINE_TEST(cTesterAADCSensors,
            TestUS,
            "3.5",
            "TestUS",
            "This test makes sure, that the 'US' pin is present. After that some US data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive arduino data and convert them into US data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoSensorTest("ultrasonic_sensors", ID_ARD_SENS_US);
}





DEFINE_TEST(cTesterAADCSensors,
            TestVoltage,
            "3.6",
            "TestVoltage",
            "This test makes sure, that the 'Voltage' pin is present. After that some voltage data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive arduino data and convert them into voltage data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoSensorTest("system_voltage", ID_ARD_SENS_VOLTAGE);
}



/**
DEFINE_TEST_INACTIVE(cTesterAADCSensors,
            TestPHOTO,
            "3.7",
            "TestPhoto",
            "This test makes sure, that the 'Photo' pin is present. After that some photo data"\
            "will be send to the filter and the filter output will be compared to some reference data.",
            "Pin can be found and the output data are as expected.",
            "See above",
            "none",
            "The filter must be able to receive arduino data and convert them into photo data.",
            "automatic")
{
    // steering angle pin with expected frameID
    return DoSensorTest("steerAngle", ID_ARD_SENS_PHOTO);
}
**/
