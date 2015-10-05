#include "mosmemoutput.h"
#include <sys/time.h>
#include <errno.h>

bool MOSharedMemory::init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "Output");

  vp.get("nb_objects",_nb_objects,1);

  key_t key;
  int shmflg;
  int shmid;
  void *shmdata;

  int num = 4 + 1;
  int size = _nb_objects * (num * sizeof(double) + sizeof(pthread_mutex_t));

  char keypath[255] = "/tmp\0";
  int keyid;
  vp.get("key_path", keypath, 255);
  vp.get("key_id", keyid, 98);
  if ((key=ftok(keypath, keyid)) == (key_t)-1) {
    EOUT("Failed to get IPC key (path " << keypath << ", ID " << keyid << ")");
    return false;
  }
  IOUT("IPC key: " << key);

  shmflg = SHM_R | SHM_W;
  if ((shmid = shmget(key, size, (shmflg | IPC_CREAT))) < 0) {
    EOUT("Failed to get IPC ID: " << shmid);
    return false;
  }
  IOUT("IPC ID: " << shmid);

  if ((shmdata = shmat(shmid, NULL, 0)) == (char*) -1) {
    EOUT("shmat");
    return false;
  }

  _mutexes = (pthread_mutex_t*)shmdata;
  _buffer = (double*)((char*)shmdata + _nb_objects * sizeof(pthread_mutex_t));

  for (int i=0; i<_nb_objects; i++) {
    pthread_mutex_init(&_mutexes[i],NULL);
  }

  gettimeofday(&_start, NULL);
  return true;
}

bool MOSharedMemory::send (std::vector<TapirObject>& objects, const timeval& time)
{
  if (objects.size() < 1) return false;
  _runtime = 1000. * (time.tv_sec-_start.tv_sec) + (time.tv_usec-_start.tv_usec) / 1000.;

  for (int i=0;i<_nb_objects;i++) {
    pthread_mutex_lock(&_mutexes[i]);

    _buffer[i*5+0]  = objects[i].x;
    _buffer[i*5+1]  = objects[i].y;
    _buffer[i*5+2]  = objects[i].size;
    _buffer[i*5+3]  = objects[i].angle;
    _buffer[i*5+4]  = _runtime;
  
    pthread_mutex_unlock(&_mutexes[i]);
  }

  return true;
}

REGISTER(Output, MOSharedMemory, "Writes multiple objects to shared memory.");
