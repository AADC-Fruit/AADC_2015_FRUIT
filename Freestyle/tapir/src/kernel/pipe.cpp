#include "pipe.h"
#include "sys/time.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


using namespace Tapir;

int Pipe::check ()
{
  if (!_use_pipe)
    return 0;

  FD_ZERO(&_rfds);
  FD_SET(_pipe, &_rfds);
  _pipe_tv.tv_sec  = 0;
  _pipe_tv.tv_usec = 1;

  if (select(_pipe+1, &_rfds, NULL, NULL, &_pipe_tv) < 1)
    return 0;
  _pipe_len = read(_pipe, &_pipe_buf, sizeof(_pipe_buf));
  return _pipe_len;
}

bool Pipe::get_cmd (char& cmd, char* params)
{
  if (!Pipe::check() < 1)
    return false;
  cmd = _pipe_buf[0];
  strcpy(params, _pipe_len < 2 ? "\0" : &_pipe_buf[2]);

  strcpy(_pipe_buf, "\0");
  Pipe::push();
  return true;
}

int Pipe::push ()
{
  if (!_use_pipe)
    return 0;

  int res = write(_pipe, &_pipe_buf, sizeof(_pipe_buf));
  //if (res == -1)
  //  EOUT("Could not write to pipe: " << strerror(errno));
  return res;
}

bool Pipe::init (const char* pipename)
{
  // set up pipe
  _use_pipe = true;

    int pipeCreation = mkfifo(pipename, 0666);
    if (pipeCreation < 0) {
      if (errno == EEXIST) {
        IOUT("Pipe " << pipename << " already exists.");
      } else {
        EOUT("Could not create pipe " << pipename << ": " << strerror(errno));
      }
    } else {
      IOUT("Pipe " << pipename << " created.");
    }

    //unlink(pipename);
    //mkfifo(pipename, 0666);
    if ((_pipe=open(pipename, O_NONBLOCK)) == -1) {
      EOUT("Can't open pipe.");
      return false;
    }
  return true;
}
