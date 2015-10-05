/******************************************************************************
Copyright (C) 2008 Matteo Lucarelli

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
******************************************************************************/

// that's the implementation file for fl_cv displays - comment and use in .h

#include "Fl_cvDisplay.h"

// Fl_cvRichDisplayWindow /////////////////////////////////////////////////////////

Fl_cvRichDisplayWindow::Fl_cvRichDisplayWindow(int X, int Y,int W,int H,const char *label,IplImage *image):Fl_Double_Window(X,Y,W,H,label)
{
	m_Display=new Fl_cvRichDisplay(5,5,W-10,H-10);

	add(m_Display);
	if (image) m_Display->SetImage(image);
	resizable(m_Display);
	show();
}

Fl_cvRichDisplayWindow::~Fl_cvRichDisplayWindow()
{
	delete m_Display;
}

void Fl_cvRichDisplayWindow::SetImage(IplImage *image, bool swap)
{
	m_Display->SetImage(image,swap);
}

// Fl_cvRichDisplay //////////////////////////////////////////////////////////////

Fl_cvRichDisplay::Fl_cvRichDisplay(int x,int y,int w,int h):Fl_Scroll(x,y,w,h,0)
{
	m_Display = new Fl_cvDisplay(x,y,w-40,h-40);
	this->add(m_Display);
}

Fl_cvRichDisplay::~Fl_cvRichDisplay()
{
	delete m_Display;
}

void Fl_cvRichDisplay::SetImage(IplImage *image, bool swap)
{
	m_Display->SetImage(image,swap);
	m_Display->size(image->width/m_Display->getZoom(),image->height/m_Display->getZoom());
	m_Display->damage(FL_DAMAGE_ALL);
}

int Fl_cvRichDisplay::handle(int event)
{
	switch ( event ) {
		// need to be catched to catch keyboard events
		case FL_FOCUS:
		case FL_UNFOCUS:
			return 1;
			break;
		case FL_KEYDOWN:
		case FL_MOUSEWHEEL:
			if ((((Fl::event_key()==43)&&Fl::event_key(43))||((Fl::event_key()==FL_KP+43)&&Fl::event_key(FL_KP+43)))||(Fl::event_key()==65260)){

				m_Display->zoomIn(true);
				damage(FL_DAMAGE_ALL);
				return 1;
			}
			else if ((((Fl::event_key()==45)&&Fl::event_key(45))||((Fl::event_key()==FL_KP+45)&&Fl::event_key(FL_KP+45)))||(Fl::event_key()==65261)){

				position(0,0); // to avoid images off the screem
				m_Display->zoomOut(true);
				damage(FL_DAMAGE_ALL);
				return 1;
			}
			else return Fl_Scroll::handle(event);
			break;
	}

	return Fl_Scroll::handle(event);
}

// Fl_cvDisplay /////////////////////////////////////////////////////////////////////////////////

Fl_cvDisplay::Fl_cvDisplay(int x,int y,int w,int h, uchar r, uchar g, uchar b):Fl_Box(x,y,w,h,0)
{
	m_Image=NULL;
	m_Zoom=1;
	mR=r;
	mG=g;
	mB=b;
}

Fl_cvDisplay::~Fl_cvDisplay()
{
}

void Fl_cvDisplay::SetImage(IplImage *image, bool swap)
{
	m_Image=image;
	
	// solo colore
	if ((m_Image->nChannels!=1)&&swap)
		cvConvertImage(m_Image,m_Image,CV_CVTIMG_SWAP_RB );
}

void Fl_cvDisplay::draw()
{
	if (!m_Image) return;
	
	fl_push_clip(x()+1,y()+1,w()-2,h()-2);
	
	// background
	fl_rectf(x()+1,y()+1,w()-2,h()-2,mR,mG,mB);
	
	fl_draw_image((uchar *)m_Image->imageData,
		x()+1, 
		y()+1,
		m_Image->width/m_Zoom, 
		m_Image->height/m_Zoom,
		m_Image->nChannels*m_Zoom,
		m_Image->widthStep*m_Zoom);
		
	fl_pop_clip();
}

void Fl_cvDisplay::zoomOut(bool adaptsize)
{
	if (m_Zoom<32){
		m_Zoom *= 2;
		if (adaptsize) size(m_Image->width/m_Zoom,m_Image->height/m_Zoom);
	}
	damage(FL_DAMAGE_ALL);
}

void Fl_cvDisplay::zoomIn(bool adaptsize)
{
	if (m_Zoom>1){
		m_Zoom /= 2;
		if (adaptsize) size(m_Image->width/m_Zoom,m_Image->height/m_Zoom);
	}
	damage(FL_DAMAGE_ALL);
}
