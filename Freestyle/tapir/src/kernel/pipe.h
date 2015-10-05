#ifndef _TAPIR_PIPE_H_
#define _TAPIR_PIPE_H_

#include "global.h"

namespace Tapir
{

class Pipe
{
 protected:
  bool _use_pipe;
  int _pipe, _pipe_len;
  char _pipe_buf[MAX_STR_LEN];
  struct timeval _pipe_tv;
  fd_set _rfds;
 public:
  Pipe () {_use_pipe = false;};
  ~Pipe () {};
  bool get_cmd (char& cmd, char* params);
  int check ();
  int push ();
  inline const char* buffer () {return _pipe_buf;};
  void buffer (const char* buf, const int len) {memcpy(_pipe_buf, buf, len * sizeof(char));}; // TODO not safe, need to check length
  bool init (const char* pipename);
};

};

#endif
