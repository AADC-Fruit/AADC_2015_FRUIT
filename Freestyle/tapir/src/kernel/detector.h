#ifndef _TAPIR_DETECTOR_H_
#define _TAPIR_DETECTOR_H_

#include "global.h"
#include "modules.h"
#include <vector>

/** @defgroup DETECTOR Detector Modules
  * Interface for object detection modules.
  * Detector modules receive raw camera input, and are
  * expected to generate a list of visible objects from
  * them. */

namespace Tapir
{

/** Interface for object detection modules.
  * Detector modules receive raw camera input, and are
  * expected to generate a list of visible objects from
  * them. */
class Detector : public TapirModule
{
 public:
  Detector () {};
  ~Detector () {};

  /** Searches for objects in the current image
    * NOTE: this function should return false only if object detection failed entirely
    * due to an error, not if it merely failed to find any images because none were visible.
    * \todo document parameters when final
    * \return false if detection was impossible, true if detection completed */
  virtual bool detect (IplImage** imgs, std::vector<TapirObject>& objects) {return true;};

  virtual bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter) {return true;};
  virtual void deinit () {};
  virtual void notify (const long int cmd, const char* params) {};
};

};

#endif
