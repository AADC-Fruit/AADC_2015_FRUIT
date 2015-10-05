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
* $Author:: spiesra $  $Date:: 2014-09-16 11:29:51#$ $Rev:: 26093   $
**********************************************************************/

/*! \brief Roadsign Detection
 *         
 *  Dieser Filter liest zunächst Verkehrszeichen als Bilder aus dem in den Eigenschaften übergebenen Pfad ein. Im Live-Modus durchsucht er das an dem InputPin übergebene Bild nach Bereichen mit den signifikanten Rot- oder Blauwerten der Verkehrszeichen in Deutschland. Diese erkannten Bereichen vergleicht er dann mit den eingelesenen Verkehrszeichen und gibt ab einer bestimmten die Nummer des erkannten Zeichens aus.
 */
#ifndef _ROADSIGNDETECTIONFILTER_HEADER
#define _ROADSIGNDETECTIONFILTER_HEADER
#define OID_ADTF_ROADSIGNDETECTIONFILTER "adtf.aadc.roadsignDetectionFilter"

//! class for filter ofroad sign detection
	/*!
	this filter was developed as prototyp for the road sign detection. It can be used as basic implemention for further work. It loads images in the defined source and is used by the update member function
	The processing in this class is based on the code published under https://sites.google.com/site/mcvibot2011sep/home. For further documentation refer to this site.
	*/
class cRoadsignDetectionFilter : public cFilter
{
	 ADTF_DECLARE_FILTER_VERSION(OID_ADTF_ROADSIGNDETECTIONFILTER, "AADC Roadsign Detection Filter", adtf::OBJCAT_Tool, "AADC Roadsign Detection Filter", 1, 0, 0, "Beta Version");	
    public:
        cRoadsignDetectionFilter(const tChar* __info);
        virtual ~cRoadsignDetectionFilter();
		tResult Init(tInitStage eStage, __exception);
		tResult Connect(IPin* pSource, const tChar* strDestname, ucom::IException** __exception_ptr);
    
	public:
        tResult OnPinEvent(adtf::IPin* pSource,
                                   tInt nEventCode,
                                   tInt nParam1,
                                   tInt nParam2,
                                   adtf::IMediaSample* pMediaSample);


	protected:
		cVideoPin m_oPinInputVideo;					/**< input Pin for video */ 
		cVideoPin m_oPinOutputVideo;				/**< output Pin for video */
		cOutputPin m_oPinSignNumber;				/**< output Pin for detected Sign as tInt */
		virtual tResult Start(__exception);
		virtual tResult Stop(__exception);
		tResult Shutdown(tInitStage eStage, __exception = NULL);


		UCOM_DECLARE_TIMING_SPOT(m_oProcessStart)
        UCOM_DECLARE_TIMING_SPOT(m_oProcessEnd)

	private:

		roadsignDetection::IRoadsignDetection *interfaceClass;	/**< pointer to interface class */
		tBool             m_bSampleVideoUpdated;				/**< flag if new data available*/
		adtf_util::cImage  m_oImage;							/**< image container for input image*/
		tBitmapFormat      m_sInputFormat;						/**< bitmapformat of input image*/
		tBitmapFormat      m_sOutputFormat;						/**< bitmapformat of output image*/
			
		tBool m_bDebugModeEnabled;								/**< indicates wheter information is printed to the console or not*/
		
		tResult ProcessVideo(adtf::IMediaSample* pSample);
		/*! function to set the m_sProcessFormat and the  m_sInputFormat variables
		@param pFormat the new format for the input pin
		*/
		tResult UpdateImageFormat(const tBitmapFormat* pFormat);
		
		
		tInt m_RoadSignResult;									/**< zuletzt erkanntes Verkehrszeichen*/
		

};
#endif //_ROADSIGNDETECTIONFILTER_HEADER
