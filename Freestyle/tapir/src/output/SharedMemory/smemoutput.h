#ifndef _SHAREDMEMORY_OUTPUT_H_
#define _SHAREDMEMORY_OUTPUT_H_

#include "output.h"
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h>

using namespace Tapir;

/** Writes object data to shared memory.
  * Clients can access the data by using the same key path and ID
  * as the server. Their values can be defined by the string
  * \b key_path and the integer \b key_id, respectively.
  *
  * @ingroup OUTPUT
  * @author Thomas Lampe, Christian Mueller */
class SharedMemory : public Output
{
 public:
  bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  bool send (std::vector<TapirObject>& objects, const timeval& time);

 protected:
  bool *_access;
  double *_buffer1, *_buffer2;
  double *_time1, *_time2;
  double _runtime;
  timeval _start;
};

#endif
