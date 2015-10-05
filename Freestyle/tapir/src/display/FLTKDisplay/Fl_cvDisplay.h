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

//-----------------------------------------------------------------------------
// Fl_cv display is a fltk display to show openCV images
// author: Matteo Lucarelli
// date: 4/10/08
//-----------------------------------------------------------------------------

// NOTE: you must call Fl::visual(FL_RGB); before build the object 
//       and Fl::run() of Fl::check() to make it alive

#ifndef FL_CVDISPLAY_H_INCLUDED
#define FL_CVDISPLAY_H_INCLUDED

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Double_Window.H>

#include "opencv/highgui.h"

// Three class are defined here
// Fl_cvDisplay: simple fltk box to show opencv images
// Fl_cvRichDisplay: box with scrollbars and zoom (mouse wheel or keyboard)
// Fl_cvRichDisplayWindow: a complete window with rich display inside

// simple box with OpenCV image loading and code-driven zoom
// inherites all the Fl_Box methods and properties
class Fl_cvDisplay : public Fl_Box
{

public:  

	// X,Y are the coordinates in window reference
	// W.H are width and height in pixel
	// r,g,b is the background color
	Fl_cvDisplay(int X, int Y,int W,int H, uchar r=0, uchar g=0, uchar b=0);
	virtual ~Fl_cvDisplay();

	// image is not copied (cannot be temporary)
	// color images are modified due to BGR of IplImage
	// use swap=false if imaged is yet swapped
	void SetImage(IplImage *image, bool swap=true);
	
	// zoom limit is 1:1 / 1:32
	// adaptsize modifies the box size to fit the image
	void zoomOut(bool adaptsize=false);
	void zoomIn(bool adaptsize=false);
	
	// return current zoom value
	int getZoom(){return m_Zoom;};
  
protected:

	void draw();
	IplImage *m_Image;
	
	int m_Zoom;
	uchar mR,mG,mB;
};

// rich display box with scrollbars and zoom keys (+/-)
// inherites all the Fl_Scroll methods and properties
class Fl_cvRichDisplay : public Fl_Scroll
{

public:
	
	// X,Y are the coordinates in window reference
	// W.H are width and height in pixel
	Fl_cvRichDisplay(int X, int Y,int W,int H);
	virtual ~Fl_cvRichDisplay();
	
	// image is not copied (cannot be temporary)
	// color images are modified due to BGR of IplImage
	// use swap=false if imaged is yet swapped
	void SetImage(IplImage *image, bool swap=true);
 
protected:
	
	Fl_cvDisplay* m_Display;
	int handle(int event);
};

// window containing a Fl_cvRichDisplay
// inherites all the Fl_Double_Window methods and properties
class Fl_cvRichDisplayWindow : public Fl_Double_Window
{

public:
	
	// X,Y are the coordinates in window reference
	// W.H are width and height in pixel
	// label is the window title
	Fl_cvRichDisplayWindow(int X, int Y,int W,int H,const char *label = 0,IplImage *image=NULL);
	virtual ~Fl_cvRichDisplayWindow();
	
	// image is not copied (cannot be temporary)
	// color images are modified due to BGR of IplImage
	// use swap=false if imaged is yet swapped
	void SetImage(IplImage *image, bool swap=true);
 
protected:
	
	Fl_cvRichDisplay* m_Display;
};

#endif //FL_CVDISPLAY_H_INCLUDED


