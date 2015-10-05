#ifndef _GYRO_YAW_EXTRACTOR_H_
#define _GYRO_YAW_EXTRACTOR_H_

#define OID_ADTF_YAW_EXTRACTOR_FILTER "fruit.dev.yaw_extractor_filter"

using namespace adtf;
/*!
* This is filter converts the gyro's signals to a usable yaw angle value in degrees and radians
*/
class YawExtractorFilter : public adtf::cFilter
{
    ADTF_FILTER(OID_ADTF_YAW_EXTRACTOR_FILTER, "FRUIT Yaw Extractor Filter", adtf::OBJCAT_DataFilter);
    public:
        YawExtractorFilter(const tChar* __info);
        virtual ~YawExtractorFilter();
    
    protected: // overwrites cFilter
        tResult Init(tInitStage eStage, __exception = NULL);
        tResult Start(__exception = NULL);
        tResult Stop(__exception = NULL);
        tResult Shutdown(tInitStage eStage, __exception = NULL);
        tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);
    
    private:
        /*! creates all the output pins */
        tResult CreateOutputPins(__exception = NULL);
        /*! creates all the input pins */
        tResult CreateInputPins(__exception = NULL);
        
        /*! calculates the euler angles from the quaternion
        @param yaw_radians pointer to the yaw angle in radians
        @param yaw_degrees pointer to the yaw angle in degrees
        */
        tResult calculateYaw(tFloat32 &yaw_radians, tFloat32 &yaw_degrees); 
        /*! this function transmits to calculated angle
        @param sampleTimeStamp timestamp of the sample
        @param timeStampValue
        @param yaw_radians yaw value in radians
        @param yaw_degrees yaw value in degrees
        */
        tResult TransmitAngles(tTimeStamp sampleTimeStamp, const tTimeStamp timeStampValue, const tFloat32 yaw_radians, const tFloat32 yaw_degrees);
        
        /*! quaternion w - component */
        tFloat32 qW;
        /*! quaternion x - component */
        tFloat32 qX;
        /*! quaternion y - component */
        tFloat32 qY;
        /*! quaternion z - component */
        tFloat32 qZ;
        /*! input pin for quaternion w - component */
        cInputPin input_qW_;
        /*! input pin for  quaternion x - component */
        cInputPin input_qX_;
        /*! input pin for quaternion y - component */
        cInputPin input_qY_;
        /*! input pin for quaternion z - component */
        cInputPin input_qZ_;
        /*! output pin for yaw in radians */
        cOutputPin output_yaw_radians_;
        
        /*! Coder Descriptor for the pins*/
        cObjectPtr<IMediaTypeDescription> m_pCoderDescSignal;
};

//*************************************************************************************************

#endif // _GYRO_YAW_EXTRACTOR_H_
