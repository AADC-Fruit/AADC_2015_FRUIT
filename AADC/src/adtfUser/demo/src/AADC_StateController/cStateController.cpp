/**
Copyright (c) 
Audi Autonomous Driving Cup. All rights reserved.
 
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: �This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.�
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS �AS IS� AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


**********************************************************************
* $Author:: forchhe#$  $Date:: 2014-09-11 10:10:54#$ $Rev:: 25921   $
**********************************************************************/
#include "stdafx.h"
#include "cStateController.h"


ADTF_FILTER_PLUGIN("AADC State Controller", __guid, cStateController);

#define SC_PROP_FILENAME "ManeuverFile"


cStateController::cStateController(const tChar* __info) : cFilter(__info), m_bDebugModeEnabled(tFalse), m_hTimer(NULL)
{
    SetPropertyBool("Debug Output to Console",false);

    SetPropertyStr(SC_PROP_FILENAME, "");
    SetPropertyBool(SC_PROP_FILENAME NSSUBPROP_FILENAME, tTrue); 
    SetPropertyStr(SC_PROP_FILENAME NSSUBPROP_FILENAME NSSUBSUBPROP_EXTENSIONFILTER, "XML Files (*.xml)");
    SetPropertyStr(SC_PROP_FILENAME NSSUBPROP_DESCRIPTION, "The maneuver file that should be processed.");
}

cStateController::~cStateController()
{
}

tResult cStateController::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    // pins need to be created at StageFirst
    if (eStage == StageFirst)    
    {
        
        cObjectPtr<IMediaDescriptionManager> pDescManager;
        RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                             IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                             (tVoid**)&pDescManager,
                                             __exception_ptr));
        /*
        * the MediaDescription for <struct name="tJuryNotAusFlag" .../> has to exist in a description file (e.g. in $ADTF_DIR\description\ or $ADTF_DIR\src\examples\src\description
        * before (!) you start adtf_devenv !! if not: the Filter-Plugin will not loaded because cPin.Create() and so ::Init() failes !
        */

        // input jury struct
        tChar const * strDesc1 = pDescManager->GetMediaDescription("tJuryStruct");
        RETURN_IF_POINTER_NULL(strDesc1);
        cObjectPtr<IMediaType> pType1 = new cMediaType(0, 0, 0, "tJuryStruct", strDesc1, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
        RETURN_IF_FAILED(m_JuryStructInputPin.Create("Jury_Struct", pType1, this));
        RETURN_IF_FAILED(RegisterPin(&m_JuryStructInputPin));
        RETURN_IF_FAILED(pType1->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescJuryStruct));
        
        // output driver struct
        tChar const * strDesc2 = pDescManager->GetMediaDescription("tDriverStruct");
        RETURN_IF_POINTER_NULL(strDesc2);
        cObjectPtr<IMediaType> pType2 = new cMediaType(0, 0, 0, "tDriverStruct", strDesc2, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
        RETURN_IF_FAILED(m_DriverStructOutputPin.Create("Driver_Struct", pType2, this));
        RETURN_IF_FAILED(RegisterPin(&m_DriverStructOutputPin));
        RETURN_IF_FAILED(pType2->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescDriverStruct));

        // inputs for set state
        tChar const * strDescignalValue = pDescManager->GetMediaDescription("tBoolSignalValue");    
        RETURN_IF_POINTER_NULL(strDescignalValue);        

        cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tBoolSignalValue", strDescignalValue,IMediaDescription::MDF_DDL_DEFAULT_VERSION);    
        RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSetStateError)); 
        RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSetStateRunning));    
        RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSetStateComplete));    
        RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSetStateReady));   
        RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescRestartSection)); 
        RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescIncrementManeuver)); 
    
        RETURN_IF_FAILED(m_StateReadyInputPin.Create("Set_State_Ready",pTypeSignalValue, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_StateReadyInputPin));
        
        RETURN_IF_FAILED(m_StateRunningInputPin.Create("Set_State_Running",pTypeSignalValue, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_StateRunningInputPin));
        
        RETURN_IF_FAILED(m_StateCompleteInputPin.Create("Set_State_Complete",pTypeSignalValue, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_StateCompleteInputPin));
        
        RETURN_IF_FAILED(m_StateErrorInputPin.Create("Set_State_Error",pTypeSignalValue, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_StateErrorInputPin));

        RETURN_IF_FAILED(m_StateIncrementManeuverInputPin.Create("Increment_Maneuver_ID",pTypeSignalValue, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_StateIncrementManeuverInputPin));

        RETURN_IF_FAILED(m_StateRestartSectionInputPin.Create("Restart_Section",pTypeSignalValue, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_StateRestartSectionInputPin));
        
        m_bDebugModeEnabled = GetPropertyBool("Debug Output to Console");

        m_i16CurrentManeuverID = 0;

        m_state = stateCar_STARTUP; 

    }
    else if(eStage == StageGraphReady)
    {
        if IS_OK(loadManeuverList())
        {
            m_i16SectionListIndex = 0;
            m_i16ManeuverListIndex = 0;
        }
        else
        {
            m_i16SectionListIndex = -1;
            m_i16ManeuverListIndex =-1;
        }
       
    }
    RETURN_NOERROR;
}

tResult cStateController::Start(__exception)
{
    return cFilter::Start(__exception_ptr);
}

tResult cStateController::Run(tInt nActivationCode, const tVoid* pvUserData, tInt szUserDataSize, ucom::IException** __exception_ptr/* =NULL */)
{
    if (nActivationCode == IRunnable::RUN_TIMER)
    {
        SendState(m_state,m_i16CurrentManeuverID);
    }   
    
    return cFilter::Run(nActivationCode, pvUserData, szUserDataSize, __exception_ptr);
}

tResult cStateController::Stop(__exception)
{
    __synchronized_obj(m_oCriticalSectionTimerSetup);
    
    destroyTimer(__exception_ptr);

    return cFilter::Stop(__exception_ptr);
}

tResult cStateController::Shutdown(tInitStage eStage, __exception)
{     
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult cStateController::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample)
{

    RETURN_IF_POINTER_NULL(pMediaSample);
    RETURN_IF_POINTER_NULL(pSource);

    if (nEventCode == IPinEventSink::PE_MediaSampleReceived  )
    {      
        //process the request to set state error pin
        if (pSource == & m_StateErrorInputPin)
        {
            tBool bValue = tFalse;
            {   // focus for sample read lock
                __adtf_sample_read_lock_mediadescription(m_pCoderDescSetStateError,pMediaSample,pCoder);
                pCoder->Get("bValue", (tVoid*)&bValue);            
            }
            if (bValue) changeState(stateCar_ERROR);
        }
        //process the request to set state running pin
        else if (pSource == & m_StateRunningInputPin)
        {
            tBool bValue = tFalse;
            {   // focus for sample read lock
                __adtf_sample_read_lock_mediadescription(m_pCoderDescSetStateRunning,pMediaSample,pCoder);
                pCoder->Get("bValue", (tVoid*)&bValue);            
            }
            if (bValue) changeState(stateCar_RUNNING);
        }
        //process the request to set state stop pin
        else if (pSource == & m_StateCompleteInputPin)
        {
            tBool bValue = tFalse;
            {   // focus for sample read lock
                __adtf_sample_read_lock_mediadescription(m_pCoderDescSetStateComplete,pMediaSample,pCoder);
                pCoder->Get("bValue", (tVoid*)&bValue);            
            }
            if (bValue) changeState(stateCar_COMPLETE);
        }
        //process the request to set state ready pin
        else if (pSource == & m_StateReadyInputPin)
        {
            tBool bValue = tFalse;
            {   // focus for sample read lock
                __adtf_sample_read_lock_mediadescription(m_pCoderDescSetStateReady,pMediaSample,pCoder);
                pCoder->Get("bValue", (tVoid*)&bValue);            
            }
            if (bValue) changeState(stateCar_READY);
        }
        //process the increment of maneuver id pin
        else if (pSource == &m_StateIncrementManeuverInputPin)
        {
            tBool bValue = tFalse;
            {   // focus for sample read lock
                __adtf_sample_read_lock_mediadescription(m_pCoderDescSetStateReady,pMediaSample,pCoder);
                pCoder->Get("bValue", (tVoid*)&bValue);            
            }
            if (bValue) incrementManeuverID();
        }

        //process the increment of restart section pin
        else if (pSource == &m_StateRestartSectionInputPin)
        {
            tBool bValue = tFalse;
            {   // focus for sample read lock
                __adtf_sample_read_lock_mediadescription(m_pCoderDescSetStateReady,pMediaSample,pCoder);
                pCoder->Get("bValue", (tVoid*)&bValue);            
            }
            if (bValue) resetSection();            
        }


        //process the request to the jury struct input pin
        else if (pSource == &m_JuryStructInputPin) 
        {
            tInt8 i8ActionID = -2;
            tInt16 i16entry = -1;
            
            {   // focus for sample read lock
                __adtf_sample_read_lock_mediadescription(m_pCoderDescJuryStruct,pMediaSample,pCoder);

                pCoder->Get("i8ActionID", (tVoid*)&i8ActionID);
                pCoder->Get("i16ManeuverEntry", (tVoid*)&i16entry);              
            }
            //change the state depending on the input
            // action_GETREADY --> stateCar_READY
            // action_START --> stateCar_RUNNING
            // action_STOP --> stateCar_STARTUP
            switch (juryActions(i8ActionID))
            {
                case action_GETREADY:
                    if(m_bDebugModeEnabled)  LOG_INFO(cString::Format("State Controller: Received: Request Ready with maneuver ID %d",i16entry));
                    changeState(stateCar_READY);
                    setManeuverID(i16entry);
                    break;
                case action_START:
                    if(m_bDebugModeEnabled)  LOG_INFO(cString::Format("State Controller: Received: Run with maneuver ID %d",i16entry));
                    if (i16entry == m_i16CurrentManeuverID)
                        changeState(stateCar_RUNNING);
                    else
                        {
                        LOG_WARNING("StateController: The id of the action_START corresponds not with the id of the last action_GETREADY");
                        setManeuverID(i16entry);
                        changeState(stateCar_RUNNING);
                        }
                    break;
                case action_STOP:
                    if(m_bDebugModeEnabled)  LOG_INFO(cString::Format("State Controller: Received: Stop with maneuver ID %d",i16entry));
                    changeState(stateCar_STARTUP);
                    break;
            }
        }
    }
    RETURN_NOERROR;

}

tResult cStateController::SendState(stateCar state, tInt16 i16ManeuverEntry)
{            
    __synchronized_obj(m_oCriticalSectionTransmit);
    
    cObjectPtr<IMediaSample> pMediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid**)&pMediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    m_pCoderDescDriverStruct->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    tInt8 bValue = tInt8(state);

    RETURN_IF_FAILED(pMediaSample->AllocBuffer(nSize));
    {   // focus for sample write lock
        __adtf_sample_write_lock_mediadescription(m_pCoderDescDriverStruct,pMediaSample,pCoder);


        pCoder->Set("i8StateID", (tVoid*)&bValue);
        pCoder->Set("i16ManeuverEntry", (tVoid*)&i16ManeuverEntry);
    }      
        
    pMediaSample->SetTime(_clock->GetStreamTime());
    m_DriverStructOutputPin.Transmit(pMediaSample);
        
    //debug output to console
    if(m_bDebugModeEnabled)  
    {
        switch (state)
            {
            case stateCar_ERROR:
                if(m_bDebugModeEnabled) LOG_INFO(cString::Format("State Controller Module: Send state: ERROR, Maneuver ID %d",i16ManeuverEntry));
                break;
            case stateCar_READY:
                if(m_bDebugModeEnabled) LOG_INFO(cString::Format("State Controller Module: Send state: READY, Maneuver ID %d",i16ManeuverEntry));
                break;
            case stateCar_RUNNING:
                if(m_bDebugModeEnabled) LOG_INFO(cString::Format("State Controller Module: Send state: RUNNING, Maneuver ID %d",i16ManeuverEntry));
                break;
            case stateCar_COMPLETE:
                if(m_bDebugModeEnabled) LOG_INFO(cString::Format("State Controller Module: Send state: COMPLETE, Maneuver ID %d",i16ManeuverEntry));                   
                break;
            case stateCar_STARTUP:
                break;
            }
    }
        
    RETURN_NOERROR;
}

tResult cStateController::loadManeuverList()
{

    m_maneuverListFile = GetPropertyStr(SC_PROP_FILENAME);
    
    if (m_maneuverListFile.IsEmpty())
    {
        LOG_ERROR("StateController: Maneuver file not found");
        RETURN_ERROR(ERR_INVALID_FILE);
    }    
    
    ADTF_GET_CONFIG_FILENAME(m_maneuverListFile);
    
    m_maneuverListFile = m_maneuverListFile.CreateAbsolutePath(".");

    //Load file, parse configuration, print the data
   
    if (cFileSystem::Exists(m_maneuverListFile))
    {
        cDOM oDOM;
        oDOM.Load(m_maneuverListFile);        
        cDOMElementRefList oSectorElems;
        cDOMElementRefList oManeuverElems;

        //read first Sector Elem
        if(IS_OK(oDOM.FindNodes("AADC-Maneuver-List/AADC-Sector", oSectorElems)))
        {                
            //iterate through sectors
            for (cDOMElementRefList::iterator itSectorElem = oSectorElems.begin(); itSectorElem != oSectorElems.end(); ++itSectorElem)
            {
                //if sector found
                tSector sector;
                sector.id = (*itSectorElem)->GetAttributeUInt32("id");
                
                if(IS_OK((*itSectorElem)->FindNodes("AADC-Maneuver", oManeuverElems)))
                {
                    //iterate through maneuvers
                    for(cDOMElementRefList::iterator itManeuverElem = oManeuverElems.begin(); itManeuverElem != oManeuverElems.end(); ++itManeuverElem)
                    {
                        tAADC_Maneuver man;
                        man.id = (*itManeuverElem)->GetAttributeUInt32("id");
                        man.action = (*itManeuverElem)->GetAttribute("action");
                        sector.maneuverList.push_back(man);
                    }
                }
    
                m_sectorList.push_back(sector);
            }
        }
        if (oSectorElems.size() > 0)
        {
            LOG_INFO("StateController: Loaded Maneuver file successfully.");
        }
        else
        {
            LOG_ERROR("StateController: no valid Maneuver Data found!");
            RETURN_ERROR(ERR_INVALID_FILE);
        }
    }
    else
    {
        LOG_ERROR("DriverFilter: no valid Maneuver File found!");
        RETURN_ERROR(ERR_INVALID_FILE);
    }
   
    RETURN_NOERROR;
}

tResult cStateController::changeState(stateCar newState)
{
    // if state is the same do nothing
    if (m_state == newState) RETURN_NOERROR;
    
    //to secure the state is sent at least one time
    SendState(newState, m_i16CurrentManeuverID);
   
    //handle the timer depending on the state
    switch (newState)
    {
    case stateCar_ERROR:
        destroyTimer();
        LOG_INFO("State ERROR reached");
        break;
    case stateCar_STARTUP:
        destroyTimer();
        LOG_INFO("State STARTUP reached");
        break;
    case stateCar_READY:
        createTimer();
        LOG_INFO(adtf_util::cString::Format("State READY reached (ID %d)",m_i16CurrentManeuverID));
        break;
    case stateCar_RUNNING:
        if (m_state!=stateCar_READY)
            LOG_WARNING("Invalid state change to Car_RUNNING. Car_READY was not reached before");
        LOG_INFO("State RUNNING reached"); 
        break;
    case stateCar_COMPLETE:
        destroyTimer();
        LOG_INFO("State COMPLETE reached");
        break;
    }
    
    m_state = newState;
    RETURN_NOERROR;
}

tResult cStateController::createTimer()
{
     // creates timer with 0.5 sec
     __synchronized_obj(m_oCriticalSectionTimerSetup);
     // additional check necessary because input jury structs can be mixed up because every signal is sent three times
     if (m_hTimer == NULL)
     {
            m_hTimer = _kernel->TimerCreate(0.5*1000000, 0, static_cast<IRunnable*>(this),
                                        NULL, NULL, 0, 0, adtf_util::cString::Format("%s.timer", OIGetInstanceName()));
     }
     else
     {
        LOG_ERROR("Timer is already running. Unable to create a new one.");
     }
     RETURN_NOERROR;
}

tResult cStateController::destroyTimer(__exception)
{
    __synchronized_obj(m_oCriticalSectionTimerSetup);
    //destroy timer
    if (m_hTimer != NULL) 
    {        
        tResult nResult = _kernel->TimerDestroy(m_hTimer);
        if (IS_FAILED(nResult))
        {
            LOG_ERROR("Unable to destroy the timer.");
            THROW_ERROR(nResult);
        }
        m_hTimer = NULL;
    }
    //check if handle for some unknown reason still exists
    else       
    {
        LOG_WARNING("Timer handle not set, but I should destroy the timer. Try to find a timer with my name.");
        tHandle hFoundHandle = _kernel->FindHandle(adtf_util::cString::Format("%s.timer", OIGetInstanceName()));
        if (hFoundHandle)
        {
            tResult nResult = _kernel->TimerDestroy(hFoundHandle);
            if (IS_FAILED(nResult))
            {
                LOG_ERROR("Unable to destroy the found timer.");
                THROW_ERROR(nResult);
            }
        }
    }

    RETURN_NOERROR;
}

tResult cStateController::incrementManeuverID()
{
    //check if list was sucessfully loaded in init    
    if (m_i16ManeuverListIndex!=-1 && m_i16SectionListIndex!=-1)
    {
        //check if end of section is reached
        if (m_sectorList[m_i16SectionListIndex].maneuverList.size()>tUInt(m_i16ManeuverListIndex+1))
        {
            //increment only maneuver index
            m_i16ManeuverListIndex++;
            m_i16CurrentManeuverID++;
        }
        else
        {
            //end of section was reached and another section is in list
            if (m_sectorList.size() >tUInt(m_i16SectionListIndex+1))
            {
                //reset maneuver index to zero and increment section list index
                m_i16SectionListIndex++;
                m_i16ManeuverListIndex=0;
                m_i16CurrentManeuverID++;
                if (m_sectorList[m_i16SectionListIndex].maneuverList[m_i16ManeuverListIndex].id !=m_i16CurrentManeuverID)
                {
                    LOG_ERROR("State Controller: inconsistancy in maneuverfile detected. Please check the file ");
                }
            }
            else
            {
                LOG_ERROR("State Controller: end of maneuverlist reached, cannot increment any more");
            }
        }
    }
    else
    {
        LOG_ERROR("State Controller: could not set new maneuver id because no maneuver list was loaded");
    }
    
    if(m_bDebugModeEnabled) LOG_INFO(adtf_util::cString::Format("Increment Manevuer ID: Sectionindex is %d, Maneuverindex is %d, ID is %d",m_i16SectionListIndex,m_i16ManeuverListIndex,m_i16CurrentManeuverID));

    RETURN_NOERROR;
}

tResult cStateController::resetSection()
{
    //maneuver list index to zero, and current maneuver id to first element in list 
    m_i16ManeuverListIndex=0;
    m_i16CurrentManeuverID = m_sectorList[m_i16SectionListIndex].maneuverList[m_i16ManeuverListIndex].id;
   
    if(m_bDebugModeEnabled) LOG_INFO(adtf_util::cString::Format("Reset section: Sectionindex is %d, Maneuverindex is %d, ID is %d",m_i16SectionListIndex,m_i16ManeuverListIndex,m_i16CurrentManeuverID));

    RETURN_NOERROR;
}

tResult cStateController::setManeuverID(tInt maneuverId)
{   
    //look for the right section id and write it to section combobox
    for(unsigned int i = 0; i < m_sectorList.size(); i++)
    {
        for(unsigned int j = 0; j < m_sectorList[i].maneuverList.size(); j++)
        {
            if(m_i16CurrentManeuverID == m_sectorList[i].maneuverList[j].id)
            {            
                m_i16SectionListIndex = i;
                m_i16ManeuverListIndex = j;
                m_i16CurrentManeuverID = maneuverId;
                if(m_bDebugModeEnabled) LOG_INFO(adtf_util::cString::Format("Sectionindex is %d, Maneuverindex is %d, ID is %d",m_i16SectionListIndex,m_i16ManeuverListIndex,m_i16CurrentManeuverID));
                break;
            }
        }
    }    
    RETURN_NOERROR;
}
