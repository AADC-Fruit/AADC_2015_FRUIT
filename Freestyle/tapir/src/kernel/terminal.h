#ifndef _TAPIR_TERMINAL_
#define _TAPIR_TERMINAL_

#include <sys/select.h>
#include <termios.h>

termios _dflags;
int _fds;

void reset_terminal_mode ()
{
  tcsetattr(_fds, TCSANOW, &_dflags);
};

void set_conio_terminal_mode ()
{
  _fds = fileno(stdin);
  termios flags;
  tcgetattr(_fds, &flags);
  tcgetattr(_fds, &_dflags);

  flags.c_lflag &= ~ICANON;
  flags.c_lflag &= ~ECHO;
  flags.c_cc[VMIN] = 0;
  flags.c_cc[VTIME] = 0;

  //cfmakeraw(&flags);
  tcsetattr(_fds, TCSANOW, &flags);
  atexit(reset_terminal_mode);
};

#endif

