/**
Copyright (c) 
Audi Autonomous Driving Cup. All rights reserved.
 
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: “This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.”
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/


// Christoph Brickl, AEV:

#ifndef _OBJECTDETECTION_FILTER_HEADER_
#define _OBJECTDETECTION_FILTER_HEADER_

#define OID_ADTF_OBJECTDETECTION  "adtf.aadc.objekterkennung"
//! class of filter for object detection
	/*!
	Der Filter Objekterkennung ermittelt auf Basis der Tiefeninformationen des "Laserscanners" der Xtion die Objekte. Mit dem Property Sensibility lässt sich die Schwelle einstellen, ab dem zwei benachbarte Punkte keine Ebene sondern einen Sprung darstellen. Die Ausgabe folgt im tObjData Struct. Dieses stellt ein L-Shape dar. Mit einem Rechten und Linken Punkt. Der Mittlere Punkt ist der Punkt zwischen den beiden Außenpunkten mit der geringsten Entfernung zur Kamera. Mit dem GCL-Offset wird der Versatz der beiden Kamera Systeme kompensiert.
	*/
class cObjectDetection : public adtf::cFilter
{

 ADTF_DECLARE_FILTER_VERSION(OID_ADTF_OBJECTDETECTION, "AADC Objectdetection Filter", adtf::OBJCAT_Tool, "AADC Objectdetection Filter", 1, 0, 0, "Beta Version");
protected:
		
		cInputPin	m_oPinInput1;		/**< Eingang für Rohdaten von Xtion_Capture_Device */

		cVideoPin	m_oPinInputVideo;	/**< Eingang für Rohdaten von Xtion_Capture_Device als video */
	
		cOutputPin	m_oOutputPin1;		/**< Ausgang für Erkannte Objekte */

		cOutputPin	m_oGLCOutput;		/**< Ausgang für GLC Video */

		tUInt16		m_Depthraw [1000];		/**< Rohdaten */

		tUInt16		m_Depth [1000];			/**< Gefilterte Daten */

		tInt32		m_DeltaDepth [1000];	/**< DeltaDepth zw. PixelX[i] zu PixelX[i+1] */
		//Bewertung von Sprüngen 
			//0=Fehlerhafte Messung; 
			//1=Depth[i] größer Depth[i+1]
			//2=Depth[i] kleiner Depth[i+1]
			//3=kein DelatDepth vorhanden
		tInt16		m_SprungBewertung [1000];	/**< Bewertung von Sprüngen */

		tInt16		m_PixelX [1000];			/**< Dazugehörige Positionen */

		tOutputModestruct m_outputMode;			/**< OutputModestruct */
			
		tObjStruct m_g_ObjStruct;				/**< Outputstruct */
		
		tBitmapFormat		m_sInputFormatVideo;	/**< bitmap format of videopin */

public:		
        cObjectDetection(const tChar*);
		virtual ~cObjectDetection();

        tResult Init(tInitStage eStage, __exception=NULL);
        tResult Shutdown(tInitStage eStage, ucom::IException** __exception_ptr=NULL);
		tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);
		tResult PropertyChanged(const char* strProperty);

private:
		//Merker für Property
		tInt m_nSensibility;			/**< sensibility property */
		tInt m_nSensibilityneg;			/**< negeative sensibility property */
		tInt m_nminBreiteObjekt;		/**< minimum width of object */
		tInt m_nGCLOffset;				/**< GCL offset */

		cObjectPtr<IMediaTypeDescriptionExt> m_pCoderDesc_Input;		/**< media descriptor for input */
		cObjectPtr<IMediaTypeDescriptionExt> m_pCoderDesc_Output;		/**< media descriptor for output */
		
		tBool m_bAlreadyRead_Input;										/**< flag for input */
		tBool m_bAlreadyRead_Output;									/**< flag for output */

		//elements for output struct
		tSize nID_i16ObjNr;					/**< number of object */
		tSize nID_i16ObjXLPos;				/**< XL position of object */
		tSize nID_i16ObjXLDept;				/**< XL Depth of object */
		tSize nID_i16ObjXMPos;				/**< XM Position of object */
		tSize nID_i16ObjXMDepth;			/**< XM depth of object */
		tSize nID_i16ObjXRPos;				/**< XR position of object */
		tSize nID_i16ObjXRDepth;			/**< XR Depth of object */
		tSize nID_i8Gueltig;				/**< valid flag */

		/*! processing
		@param pSample the input media sample
		*/
		tResult ProcessInputImage(IMediaSample* pSample);
		/*! processing
		@param pSample the input media sample
		*/
		tResult ProcessInput(IMediaSample* pSample);
		/*! processing
		*/
		tResult ProcessFound();
		/*! processing
		*/
		tResult ProcessFoundOffline();
		
		/*! processing
		*/
		tResult ProcessOutput();
		
		/*! function to set the m_sProcessFormat and the  m_sInputFormat variables
		@param pFormat the new format for the input pin
		*/
		tResult UpdateImageFormat(const tBitmapFormat* pFormat);
		/*! convert real world to projective offline
		@param nCount number of entries
		@param aRealWorld pointer to real world vector
		@param aProjective pointer to projective vector
		*/
		tResult ConvertRealWorldToProjectiveOffline(tInt32 nCount, const tFloatVector3D* aRealWorld, tFloatVector3D* aProjective);
		tResult CreateAndTransmitGCL();

};

#endif 
