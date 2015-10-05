#ifndef _TAPIR_OUTPUT_H_
#define _TAPIR_OUTPUT_H_

#include "global.h"
#include "modules.h"

/** @defgroup OUTPUT Output Modules
  * Interface for modules that assign objects to their final purpose.
  * Output modules do \e not display images; that's the job of the
  * Display module class. Instead, they receive the list of objects
  * that were selected, and can in principle do anything with them.
  * Their primary role is to send them to other programs that then
  * use the object information for some other goal. */

namespace Tapir
{

/** Interface for modules that assign objects to their final purpose.
  * Output modules do \e not display images; that's the job of the
  * Display module class. Instead, they receive the list of objects
  * that were selected, and can in principle do anything with them.
  * Their primary role is to send them to other programs that then
  * use the object information for some other goal. */
class Output: public TapirModule
{
 public:
  Output () {};
  ~Output () {};

  /** Performs an arbitrary post-detection operation with a filtered list of objects.
    * \param objects list of objects that were found by detector and selected by tracker
    * \param time time of image acquisition
    * \return false if operation failed */
  virtual bool send (std::vector<TapirObject>& objects, const timeval& time) {return true;};

  virtual bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter) {return true;};
  virtual void deinit () {};
  virtual void notify (const long int cmd, const char* params) {};
};

};

#endif
