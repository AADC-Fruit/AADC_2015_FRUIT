#ifndef _TAPIR_MODULES_H_
#define _TAPIR_MODULES_H_

namespace Tapir
{

/** Basic template for any module types.
  * Ensures that functions required by macros which are called for all modules are present. */
class TapirModule
{
 public:
  /** Initializes the module. Called once before first use of any other function.
    * \param width width of the source image
    * \param height height of the source image
    * \param fname path to the program configuration
    * \return false if initialization failed */
  virtual bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter) {return false;};

  /** Deinitializes the module. Called at program exit. */
  virtual void deinit () = 0;

  /** Forwards a user input to the module.
    * NOTE: 'q' is globally mapped to exit the program. All
    * other keys can be assigned freely.
    * \param key character to be interpreted */
  virtual void notify (const long int cmd, const char* params) = 0;
};

#define FOR_EACH_MODULE(_macro_) \
_macro_(ImageSource) \
_macro_(Detector) \
_macro_(Tracker) \
_macro_(Output) \
_macro_(Display)

};

#endif
