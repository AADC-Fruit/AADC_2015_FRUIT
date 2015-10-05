#include "smemoutput.h"
#include <sys/time.h>

bool SharedMemory::init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "Output");

  key_t key;
  int shmflg;
  int shmid;
  void *shmdata;

  int num = 4;
  int size = 2 * (num * sizeof(double) + sizeof(int)) + sizeof(bool);

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

  _access  = (bool*)shmdata;
  _time1   = (double*) ((char*)shmdata + sizeof(bool));
  _buffer1 = (double*) ((char*)shmdata + sizeof(bool) + sizeof(double));
  _time2   = (double*) ((char*)shmdata + sizeof(bool) + sizeof(double) * (num+1));
  _buffer2 = (double*) ((char*)shmdata + sizeof(bool) + sizeof(double) * (num+2));
  *_access = 0;

  gettimeofday(&_start, NULL);
  return true;
}

bool SharedMemory::send (std::vector<TapirObject>& objects, const timeval& time)
{
  if (objects.size() < 1) return false;
  _runtime = 1000. * (time.tv_sec-_start.tv_sec) + (time.tv_usec-_start.tv_usec) / 1000.;

  _buffer1[0]  = objects[0].x;
  _buffer1[1]  = objects[0].y;
  _buffer1[2]  = objects[0].size;
  _buffer1[3]  = objects[0].angle;
  *_time1 = _runtime;
  *_access = 1;

  _buffer2[0]  = objects[0].x;
  _buffer2[1]  = objects[0].y;
  _buffer2[2]  = objects[0].size;
  _buffer2[3]  = objects[0].angle;
  *_time2 = _runtime;
  *_access = 0;

  return true;
}

REGISTER(Output, SharedMemory, "Writes a single object to shared memory.");
