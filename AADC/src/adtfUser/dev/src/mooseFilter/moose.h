/**
 *
 * ADTF Moose Filter.
 *
 * @file
 * Copyright &copy; FRUIT. All rights reserved
 *
 * $Author: FRUIT $
 * $Date: 2011-06-30 16:51:21 +0200 (Thu, 30 Jun 2011) $
 * $Revision: 26514 $
 *
 * @remarks
 *
 */
#ifndef _MOOSE_FILTER_H_
#define _MOOSE_FILTER_H_

#define OID_ADTF_MOOSE_FILTER "fruit.dev.moose"


//*************************************************************************************************
class MooseFilter : public adtf::cFilter
{
    ADTF_FILTER(OID_ADTF_MOOSE_FILTER, "Moose Filter", adtf::OBJCAT_DataFilter);

protected:
    cInputPin   input_uss_front_left_;
    cInputPin   input_uss_front_right_;

    cOutputPin  output_head_lights_;
    cOutputPin  output_break_lights_;
    cOutputPin  output_turn_signal_left_;
    cOutputPin  output_turn_signal_right_;
    cOutputPin  output_steering_angle_;
    cOutputPin  output_motors_;

    tFloat32 uss_front_left_value_, uss_front_right_value_, motor_speed_, steering_angle_;
    tBool head_lights_value_, break_lights_value_, left_signal_value_, right_signal_value_;

public:
    MooseFilter(const tChar* __info);
    virtual ~MooseFilter();

private:
    tResult TransmitLight(unsigned int lightIndex, tBool state);
    tResult SetLightConfig(int cycle, tBool head, tBool breaklight, tBool left, tBool right);
    int cycle_;
    
    void MotorSpeed(int val);
    void SteeringAngle(int val);

protected:
    tResult Init(tInitStage eStage, __exception);
    tResult Shutdown(tInitStage eStage, __exception);
    /*! Coder Descriptor for the input pins*/
	  cObjectPtr<IMediaTypeDescription> m_pCoderDescSignalInput;
    cObjectPtr<IMediaTypeDescription> m_pCoderDescBoolSignalOutput;
    cObjectPtr<IMediaTypeDescription> m_pCoderDescSignalOutput;

    // implements IPinEventSink
    tResult OnPinEvent(IPin* pSource,
                       tInt nEventCode,
                       tInt nParam1,
                       tInt nParam2,
                       IMediaSample* pMediaSample);
};

//*************************************************************************************************
#endif // _MOOSE_FILTER_H_
