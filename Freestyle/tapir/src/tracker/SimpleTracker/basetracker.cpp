#include "basetracker.h"
#include <cmath>

#define CONFOPT(xxx,yyy) if (strcmp(tmp,xxx)==0) _mode=yyy
#define OBJADD(xxx) obj.xxx += objects[_i].xxx

bool BaseTracker::init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "Tracker");
  char tmp[255];
  vp.get("mode", tmp, 255);
  if (strcmp(tmp,"largest")==0) _mode = Largest;
  else if (strcmp(tmp,"nearest")==0) {
    _mode = Nearest;
    REGISTER_KEY(BaseTracker, 'n', "Choose object to the right of the current one");
    REGISTER_KEY(BaseTracker, 'p', "Choose object to the left of the current one");
    _next = 0;
  } else if (strcmp(tmp,"center")==0) _mode = Center;
  else if (strcmp(tmp,"gravity")==0) _mode = Gravity;
  else {
    EOUT("Unknown tracker mode (" << tmp << ")");
    return false;
  }
  _last.x = width / 2;
  _last.y = height / 2;
  _canvas = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
  _imgnum = imgs;
  add = 1;

  vp.get("draw_on_original", _original, true);

  return true;
}

void BaseTracker::select (IplImage** imgs, std::vector<TapirObject>& objects)
{
  cvCopy(imgs[_original?0:_imgnum-1], _canvas);
  imgs[_imgnum] = _canvas;
  if (objects.size() < 1) return;

  _pick = 0;
  switch (_mode) {

   case Largest:
   {
    double size = -1.;
    for (_i=0; _i<(int)objects.size(); _i++) {
      if (objects[_i].size > size) {
        size = objects[_i].size;
        _pick = _i;
      }
    }
    _buffer.push_back(objects[_pick]);
    break;
   }

   case Center:
   {
    TapirObject obj(0,0,0);
    for (_i=0; _i<(int)objects.size(); _i++) {
      OBJADD(x);
      OBJADD(y);
      OBJADD(size);
    }
    obj.x /= objects.size();
    obj.y /= objects.size();
    _buffer.push_back(obj);
    break;
   }

   case Gravity:
   {
    TapirObject obj(0,0,0);
    for (_i=0; _i<(int)objects.size(); _i++) {
      OBJADD(x) * objects.at(_i).size;
      OBJADD(y) * objects.at(_i).size;
      OBJADD(size);
    }
    obj.x /= obj.size;
    obj.y /= obj.size;
    _buffer.push_back(obj);
    break;
   }

   case Nearest:
   {
    // choose closest
    double dist = 1e6;
    double tmp;
    for (_i=0; _i<(int)objects.size(); _i++) {
      tmp = pow(objects[_i].x-_last.x,2)+pow(objects[_i].y-_last.y,2);
      if (tmp < dist) {
        dist = tmp;
        _pick = _i;
      }
    }

    // choose closest from closest
    if (_next != 0) {

      int nearest = _pick;
      int wrap    = _pick;
      int outer = 1e6;
      dist = -1e6;
      for (_i=0; _i<(int)objects.size(); _i++) {
        if (_i == _pick) continue;
        tmp = _next * (objects[_pick].x - objects[_i].x);
        if (tmp < 0. && tmp > dist) {
          dist = tmp;
          nearest = _i;
        }
        tmp = objects[_i].x * _next;
        if (tmp < outer) {
          outer = tmp;
          wrap = _i;
        }
      }
      _pick = nearest == _pick ? wrap : nearest;
      _next = 0;
    }

    _buffer.push_back(objects[_pick]);
    _last.x = objects[_pick].x;
    _last.y = objects[_pick].y;
    break;
   }

   default:
    EOUT("Invalid tracker mode.");
  }
  cvCircle(_canvas, cvPoint(_buffer.at(0).x,_buffer.at(0).y), 5, cvScalar(0,0,255), 5);
  _buffer.swap(objects);
  _buffer.clear();
}

void BaseTracker::notify (const long int cmd, const char* params)
{
  if (_mode == Nearest) {
    switch (cmd) {
      case 'n': _next =  1; break;
      case 'p': _next = -1; break;
      default: return;
    }
  }
}

REGISTER(Tracker, BaseTracker, "Selects a single object based on size or distance.");
