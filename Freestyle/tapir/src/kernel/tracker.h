#ifndef _TAPIR_TRACKER_H_
#define _TAPIR_TRACKER_H_

#include "global.h"
#include "modules.h"

/** @defgroup TRACKER Tracker Modules
  * Interface for modules that select objects from those found by the detector.
  * Tracker modules allow implementing certain frequently used selection
  * schemes independently from the detection algorithm; for instance, selecting
  * the largest visible object is equally applicable to colored blobs and to
  * faces. */

namespace Tapir
{

/** Interface for modules that select objects from those found by the detector.
  * Tracker modules allow implementing certain frequently used selection
  * schemes independently from the detection algorithm; for instance, selecting
  * the largest visible object is equally applicable to colored blobs and to
  * faces. */
class Tracker : public TapirModule
{
 public:
  Tracker () {};
  ~Tracker () {};

  /** Modifies the list of detected objects.
    * \param objects objects that were found by the detector */
  virtual void select (IplImage** imgs, std::vector<TapirObject>& objects) {};

  virtual bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter) {return true;};
  virtual void deinit () {};
  virtual void notify (const long int cmd, const char* params) {};
 protected:
};

};

#endif

