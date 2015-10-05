#ifndef _MULTI_TRACKER_H_
#define _MULTI_TRACKER_H_

#include "tracker.h"
#include <vector>

using namespace Tapir;

/** Meta-tracker for using several tracker modules at once.
  *
  * @ingroup TRACKER
  * @author Thomas Lampe */
class MultiTracker : public Tracker
{
 public:
  bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  void select (IplImage** imgs, std::vector<TapirObject>& objects);
  void deinit ();
  void notify (const long int cmd, const char* params);

 protected:
  std::vector<Tracker*> _slaves;
  int _i;
};

#endif
