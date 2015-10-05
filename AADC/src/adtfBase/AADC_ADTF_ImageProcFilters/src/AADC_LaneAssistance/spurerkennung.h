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

/*! \brief cSpurerkennung
 *         
•	Zur Erkennung einer Spur wird das RGB Bild der Asus Xtion eingelesen und verarbeitet
•	Weiteres Vorgehen:
•	Zuschneiden des Orginal Bildes auf die eingestellte Größe
•	Erzeugen eines Graustufen Bildes
•	Anwenden eines Schwellwertfilters
•	Kantendedektion
•	Suchen nach Kanten auf den eingestellten cm-Marken
•	Auswerten der gefundenen Punkte ob sinnvolle Linie erstellt werden kann
•	Anzeigen der Linie mit Hilfe der GLC

 */

#ifndef _Spurerkennung_FILTER_HEADER_
#define _Spurerkennung_FILTER_HEADER_

#define OID_ADTF_Spurerkennung  "adtf.aadc.Spurerkennung"

//! class for lane detection
	/*!
	This class was developed as a prototyp for a lane detection in the video stream. It can be used as a prototyp for further development.
	*/
class cSpurerkennung : public adtf::cFilter
{
	 ADTF_DECLARE_FILTER_VERSION(OID_ADTF_Spurerkennung, "AADC Spurerkennung", adtf::OBJCAT_Tool, "AADC Spurerkennung", 1, 0, 0, "Beta Version");	
protected:
//Eingang für RGB Bild
		cVideoPin		m_oVideoInputPin;		/**< the input pin for the video*/
		cOutputPin      m_oGLCOutput;			/**< the output pin for the glc data*/

public:
		/*! struct for a point 
		*/
		struct sPoint 
		{
			/*! x component of the point */
			tInt x; 
			/*! y component of the point*/
			tInt y; 
		};

		cSpurerkennung(const tChar*);
		virtual ~cSpurerkennung();
		tResult Init(tInitStage eStage, __exception=NULL);
		tResult Shutdown(tInitStage eStage, ucom::IException** __exception_ptr=NULL);
		tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);
		/*! processing
		@param pSample the input media sample
		*/tResult ProcessInput(IMediaSample* pSample);
		/*! processing the image
		*/
		tResult ProcessFound();
		/*! processing the image
		*/
		tResult ProcessOutput();
		/*! changed a property
		@param strProperty char pointer to property
		*/
		tResult PropertyChanged(const char* strProperty);
		/*! processed the image
		*/
		tResult CreateAndTransmitGCL();
		/*! searches the data
		@param points pointer to the points
		@param lfdnr the number of line for this search
		@param offset the offset for this search
		@param limit the limit for this search
		*/
		tResult Search(sPoint *points,tUInt8 *lfdnr,tUInt8 offset,tUInt8 limit);
		/*! planning
		@param points1 the coordinates of points n# 1
		@param points2 the coordinates of points n# 2
		@param lfdnr1 the number of line for points 1
		@param lfdnr2 the number of line for points 1
		*/
		tResult Planing(sPoint *points1, sPoint *points2,tUInt8 *lfdnr1,tUInt8 *lfdnr2);
		/*! updates the image format
		@param pFormat the new image format
		*/
		tResult UpdateImageFormat(const tBitmapFormat* pFormat);

		//für Search
		sPoint allpoint1[641];			/**< coordinates point n# 1*/
		tUInt8 lfdnr1;					/**< line of point n# 1*/
		sPoint allpoint2[641];			/**< coordinates point n# 2*/
		tUInt8 lfdnr2;					/**< line of point n# 2*/
		sPoint allpoint3[641];			/**< coordinates point n# 3*/
		tUInt8 lfdnr3;					/**< line of point n# 3*/
		sPoint allpoint4[641];			/**< coordinates point n# 4*/
		tUInt8 lfdnr4;					/**< line of point n# 4*/
		sPoint allpoint5[641];			/**< coordinates point n# 5*/
		tUInt8 lfdnr5;					/**< line of point n# 5*/
		sPoint allpoint6[641];			/**< coordinates point n# 6*/
		tUInt8 lfdnr6;					/**< line of point n# 6*/
		sPoint allpoint7[641];			/**< coordinates point n# 7*/
		tUInt8 lfdnr7;					/**< line of point n# 7*/
		sPoint allpoint8[641];			/**< coordinates point n# 8*/
		tUInt8 lfdnr8;					/**< line of point n# 8*/
		//Zeichen
		sPoint printingpoints[1000];	/**< coordinates for the printed points*/
		tUInt8 printingpointsnr;		/**< number of the printed points*/
		//PolyLine
		tUInt8 polylinenr;				/**< number of the poly-line*/
		tUInt8 polylinesize[100];		/**< size of the poly-line*/
		sPoint ploylinepoints[100][9];	/**< coordinates of the polyline*/

		cv::Mat Line;					/**< matrix for the line*/
		cv::Mat grey;					/**< matrix for the gray image*/
		cv::Mat greythresh;				/**< matrix for the gray threshold*/
		cv::Size cannysize;				/**< size for the canny detector*/
		cv::Mat linecanny;				/**< size for the canny lines*/

private:
		tBool firstFrame;				/**< flag for the first frame*/
		tUInt8 imagecount;				/**< counter for the imaes*/
		tBitmapFormat m_sInputFormat;	/**< bitmap format of the input image*/
		
		//Properties:
        tInt m_nFadenkreuzH;			/**< the horizontal coordinate of the center*/
        tInt m_nFadenkreuzV;			/**< the vertical coordinate of the center*/
		tInt m_nGrenzeVLinks;			/**< the vertical limit on the left side*/
		tInt m_nGrenzeVRechts;			/**< the vertical limit on the right side*/
		tInt m_nGrenzeHOben;			/**< the horizontal limit on the upper side*/
		tInt m_nGrenzeHUnten;			/**< the horizontal limit on the lower side*/
		tInt m_nthresholdvalue;			/**< the threshold value*/
		tInt m_nDiff;					/**< the difference factor*/
		tInt m_50cmMarke;				/**< coordinate of 50 cm mark*/
		tInt m_70cmMarke;				/**< coordinate of 70 cm mark*/
		tInt m_90cmMarke;				/**< coordinate of 90 cm mark*/
		tInt m_110cmMarke;				/**< coordinate of 110 cm mark*/
		tInt m_130cmMarke;				/**< coordinate of 130 cm mark*/
		tInt m_150cmMarke;				/**< coordinate of 150 cm mark*/
		tInt m_170cmMarke;				/**< coordinate of 170 cm mark*/
		tInt m_190cmMarke;				/**< coordinate of 190 cm mark*/

		
};

#endif 
