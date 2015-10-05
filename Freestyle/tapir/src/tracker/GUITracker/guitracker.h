#ifndef _GUI_TRACKER_H_
#define _GUI_TRACKER_H_

#include "tracker.h"
#include <vector>

using namespace Tapir;

struct GUITrackerElement
{
  IplImage *frame;
  int x, y, w, h, s, a, b;
};

/**
  *
  * @ingroup TRACKER
  * @author Thomas Lampe */
class GUITracker : public Tracker
{
 public:
  bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  void select (IplImage** imgs, std::vector<TapirObject>& objects);
  void notify (const long int cmd, const char* params);

 protected:
  IplImage *_canvas;
  std::vector<GUITrackerElement> _buttons;
  int _i, _imgnum, _width, _height;
  bool _active;
};

#endif
