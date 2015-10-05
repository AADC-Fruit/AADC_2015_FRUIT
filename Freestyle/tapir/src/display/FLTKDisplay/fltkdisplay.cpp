#include "fltkdisplay.h"

#define FLTK_MAX_NUM_DISPLAYS 4

bool FLTKDisplay::init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "Display");
  _ids = new int[FLTK_MAX_NUM_DISPLAYS];
  for (_i=0; _i<FLTK_MAX_NUM_DISPLAYS; _i++)
    _ids[_i] = _i;
  _displays = vp.get("ids", _ids, FLTK_MAX_NUM_DISPLAYS);
  if (_displays < 1)
    _displays = 1;

  Fl::visual(FL_RGB);
  _wMain = new Fl_Double_Window(width, _displays*height, "Kamera");
  _display = new Fl_cvRichDisplay*[_displays];
  for (_i=0; _i<_displays; _i++) {
    _display[_i] = new Fl_cvRichDisplay(0, _i*height, width, height);
    _wMain->add(_display[_i]);
  }
  _wMain->show();

  for (_i=0; _i<_displays; _i++)
    if (_ids[_i] >= imgs) {
      WOUT("Invalid frame id #" << _ids[_i]);
      _ids[_i] = -1;
  }

  _width  = width;
  _height = height;

  REGISTER_KEY(FLTKDisplay, 'h', "Hides the window");
  REGISTER_KEY(FLTKDisplay, 's', "Shows the window");

  return true;
}

bool FLTKDisplay::display (IplImage** imgs)
{
//  if (!_wMain->shown()) return true;
  for (_i=0; _i<_displays; _i++)
    if (_ids[_i] > -1 && imgs[_ids[_i]] != 0)
      _display[_i]->SetImage(imgs[_ids[_i]]);
  Fl::check();
  return true;
}

bool FLTKDisplay::get_event (char cmd[3], char* params)
{
  switch (Fl::event()) {
    case FL_PUSH:
      sprintf(params, "%d %d", Fl::event_x()%_width, Fl::event_y()%_height);
      switch (Fl::event_button()) {
        case FL_LEFT_MOUSE:   cmd[0] = 'L'; break;
        case FL_MIDDLE_MOUSE: cmd[0] = 'M'; break;
        case FL_RIGHT_MOUSE:  cmd[0] = 'R'; break;
        default: cmd[0] = '?';
      }
      break;
    case FL_KEYUP:
//      cmd[0] = Fl::event_text()[0];
//      break;
    default:
      return false;
  }
  return true;
}

void FLTKDisplay::notify (const long int cmd, const char* params)
{
  switch (cmd) {
    case 'h':
      _wMain->hide();
      break;
    case 's':
      _wMain->show();
      break;
    default:
      return;
  }
}

REGISTER(Display, FLTKDisplay, "Shows an image using the Fast Light ToolKit.");

