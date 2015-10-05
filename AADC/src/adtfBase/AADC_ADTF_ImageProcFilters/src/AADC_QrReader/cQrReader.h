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


#ifndef _QRREADER_HEADER
#define _QRREADER_HEADER
#define OID_ADTF_QRREADERFILTER "adtf.aadc.QrReaderFilter"
//! class of filter for reading qr codes
	/*!
	this filter reads QR signs from the given video stream
	*/
class cQrReader : public cFilter
{
	ADTF_DECLARE_FILTER_VERSION(OID_ADTF_QRREADERFILTER, "AADC QR Reader Filter", adtf::OBJCAT_Tool, "AADC QR Reader", 1, 0, 0, "Beta Version");	
    public:
        cQrReader(const tChar* __info);
        ~cQrReader();
		tResult Init(tInitStage eStage, __exception);
		
	public:
        tResult OnPinEvent(adtf::IPin* pSource,
                                   tInt nEventCode,
                                   tInt nParam1,
                                   tInt nParam2,
                                   adtf::IMediaSample* pMediaSample);

	protected:
		cVideoPin m_oPinInputVideo;		/**< input Pin for video */
		
		tResult Start(__exception);
		tResult Stop(__exception);
		tResult Shutdown(tInitStage eStage, __exception = NULL);

	private:
		/*! function which does the processing of the data
		@param pSample the incoming media sample
		*/
		tResult ProcessVideo(adtf::IMediaSample* pSample);

		/*! function to set the m_sProcessFormat and the  m_sInputFormat variables
		@param pFormat the new format for the input pin
		*/
		tResult UpdateImageFormat(const tBitmapFormat* pFormat);	
		
		ImageScanner  m_qrScanner;					/**< a zbar image scanner for reading qr codes*/

		string m_lastQRCode;						/**< string which holds the last detectec code*/

		tBool m_bConsoleOutputEnabled;				/**< indicates whether information is printed to the console or not */
				
		tBitmapFormat      m_sProcessFormat;		/**< holds the imageformat for processing the data */
		tBitmapFormat      m_sInputFormat;			/**< holds the imageformat of the input data/pin */	
};
#endif //_QRREADER_HEADER
