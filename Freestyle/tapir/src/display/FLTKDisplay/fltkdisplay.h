#ifndef _FLTK_DISPLAY_H_
#define _FLTK_DISPLAY_H_

#include "display.h"
#include "Fl_cvDisplay.h"

using namespace Tapir;

/** Fast-Light-Toolkit-based GUI.
  * Lightweight display that provides a single window with two joined screens.
  * The parameter \b ids contains a list of the frames to be shown.
  *
  * @ingroup DISPLAY
  * @author Matteo Lucarelli (core display), Thomas Lampe (module) */
class FLTKDisplay : public Tapir::Display
{
 public:
  FLTKDisplay () {};
  ~FLTKDisplay () {};
  bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  bool display (IplImage** imgs);
  bool get_event (char cmd[3], char* params);
  void notify (const long int cmd, const char* params);

 protected:
  Fl_cvRichDisplay **_display;
  Fl_Double_Window* _wMain;
  int *_ids, _i, _displays, _width, _height;
};

#endif

