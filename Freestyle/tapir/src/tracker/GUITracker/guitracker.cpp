#include "guitracker.h"
#include <cmath>

bool GUITracker::init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "Tracker");
  std::stringstream ss;
  char file[255];
  int pos[2];

  for (_i=0; ; _i++) {
    ss.str("");
    ss << "item_" << _i << "_file";
    if (vp.get(ss.str().c_str(), file, 255) < 1) break;

    ss.str("");
    ss << "item_" << _i << "_pos";
    if (vp.get(ss.str().c_str(), pos, 2) < 2) break;

    GUITrackerElement button;
    button.x = pos[0];
    button.y = pos[1];
    button.frame = cvLoadImage(file, CV_LOAD_IMAGE_COLOR);
    if (button.frame == 0) {
      EOUT("Cannot load file " << file << "!");
      return false;
    }

    button.w = button.frame->width;
    button.h = button.frame->height;
    button.s = button.w * button.h;
    button.a = button.x - button.w / 2;
    button.b = button.y - button.h / 2;

    _buttons.push_back(button);
  }

  REGISTER_KEY(GUITracker, 'a', "Activate GUI");
  REGISTER_KEY(GUITracker, 'd', "Deactivate GUI");

  _active = true;
  _canvas = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
  _imgnum = imgs;
  add = 1;
  _width  = width;
  _height = height;

  return true;
}

void GUITracker::select (IplImage** imgs, std::vector<TapirObject>& objects)
{
  cvCopy(imgs[_imgnum-1], _canvas);
  imgs[_imgnum] = _canvas;
  if (!_active) return;

  for (_i=0; _i<(int)_buttons.size(); _i++) {
    cvSetImageROI(_canvas, cvRect(_buttons[_i].a, _buttons[_i].b, _buttons[_i].w, _buttons[_i].h));
    cvCopy(_buttons[_i].frame, _canvas);
    objects.push_back(TapirObject(_buttons[_i].x, _buttons[_i].y, _buttons[_i].s));
  }
  cvSetImageROI(_canvas, cvRect(0, 0, _width, _height));
}

void GUITracker::notify (const long int cmd, const char* params)
{
  switch (cmd) {
    case 'a': _active = true;  break;
    case 'd': _active = false; break;
  }
}

REGISTER(Tracker, GUITracker, "Adds artificial objects for use as GUI elements.");
