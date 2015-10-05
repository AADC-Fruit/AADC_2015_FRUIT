#ifndef _SHAREDMEMORY_OUTPUT_H_
#define _SHAREDMEMORY_OUTPUT_H_

#include "output.h"
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h>
#include <pthread.h>

using namespace Tapir;

/** Writes objects data to shared memory.
  * Clients can access the data by using the same key path and ID
  * as the server. Their values can be defined by the string
  * \b key_path and the integer \b key_id, respectively.
  *
  * The number of objects must be defined 
  * \b nb_objects  
  * 
  * @ingroup OUTPUT
  * @author Xavier Bouthillier */
class MOSharedMemory : public Output
{
 public:
  bool init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  bool send (std::vector<TapirObject>& objects, const timeval& time);

 protected:
  pthread_mutex_t *_mutexes;
  double *_buffer;
  double _runtime;
  int _nb_objects;
  int _nb_attributes;
  timeval _start;
};

#endif
