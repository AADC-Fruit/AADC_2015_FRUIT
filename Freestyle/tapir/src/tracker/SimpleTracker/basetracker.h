#ifndef _BASE_TRACKER_H_
#define _BASE_TRACKER_H_

#include "tracker.h"
#include <vector>

using namespace Tapir;

/** Simple tracker that selects single objects.
  * Different variations can be chosen through the parameter \b mode:
  * \li \e largest selects the object with the highest size attribute;
  * \li \e nearest selects the object with the smallest cartesian distance
  *     from that of he object during the last program cycle; the initial
  *     object is chosen as the one closest to the center of the frame;
  * \li \e center returns a new object whose coordinates are the average
  *     of all detected objects, and whose size is their total.
  *
  * @ingroup TRACKER
  * @author Thomas Lampe */
class BaseTracker : public Tracker
{
 public:
  bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  void select (IplImage** imgs, std::vector<TapirObject>& objects);
  void notify (const long int cmd, const char* params);

 protected:
  enum {Largest, Nearest, Center, Gravity} _mode;
  std::vector<TapirObject> _buffer;
  IplImage *_canvas;
  int _pick, _i, _imgnum, _next;
  TapirObject _last;
  bool _original;
};

#endif
