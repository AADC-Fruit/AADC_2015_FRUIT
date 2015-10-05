#include "global.h"
#include "modules.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <algorithm>
#include <string>
#include "terminal.h"
#include "sys/time.h"
#include "opencv/highgui.h"

#include "imagesource.h"
#include "detector.h"
#include "output.h"
#include "display.h"
#include "tracker.h"
#include "valueparser.h"
#include "pipe.h"

#define MAX_IMAGES 10
#define MAX_CMD_ARGS 10

using namespace std;
using namespace Tapir;

Tapir::Pipe _pipe;

bool get_cmd_from_pipe (char& cmd, char* params)
{
  if (!_pipe.check() < 1)
    return false;
  const char* buf = _pipe.buffer();
  cmd = buf[0];
  strcpy(params, strlen(buf) < 2 ? "\0" : &buf[2]);

  _pipe.buffer("\0", sizeof(char));
  _pipe.push();
  return true;
}

/** Helper module to handle events for mainloop.
  * @author Thomas Lampe */
class MainModule : public TapirModule
{
 public:
  MainModule ()
  {
    quit = false;
    REGISTER_KEY(MainModule, 'q', "Quit Tapir");
    REGISTER_HIDDEN_KEY(MainModule, '*');
    REGISTER_HIDDEN_KEY(MainModule, '?');
  };
  inline void deinit () {};
  inline void notify (const long int cmd, const char* params)
  {
    switch (cmd) {
    case 'q':
      quit = true;
      break;
    case '?':
      IOUT("Key mapping:" << std::endl << KeyRegistry::instance()->list.str());
      break;
    default:
      EOUT("Invalid command " << cmd);
    }
  };
  bool quit;
};

#define REGISTER_DUMMY_AND_DECLARE(_type_) \
REGISTER(_type_,_type_,"Dummy"); \
Tapir::_type_ *_##_type_;
FOR_EACH_MODULE(REGISTER_DUMMY_AND_DECLARE);
MainModule *_main;

IplImage *_images[MAX_IMAGES];
std::vector<TapirObject> _objects;
timeval _time;
bool _flip = false;
bool _norm = false;

void sighandler (int sig)
{
  EOUT("Caught SIGABORT, will quit Tapir now!");
  _main->quit = true;
};

int main (int argc, char **argv)
{
  // usage help
#define GENERATE_LOWERCASE(_type_) \
  std::string _type_##Section = #_type_; \
  std::transform(_type_##Section.begin(), _type_##Section.end(), _type_##Section.begin(), ::tolower);
  FOR_EACH_MODULE(GENERATE_LOWERCASE);
#define PRINT_HELP(_type_) \
  std::cout << "  To get a list of available " << _type_##Section.c_str() << " modules: " << argv[0] << " --list_" << _type_##Section.c_str() << "s" << endl;
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <params.cfg>" << endl;
    FOR_EACH_MODULE(PRINT_HELP);
    return 0;
  }

  // module listings
#define LIST_MODULES(_type_) \
  if (strcmp(argv[1], string("--list_").append(_type_##Section).append("s").c_str())==0  || \
      strcmp(argv[1], string("--list_").append(_type_##Section).c_str())==0 ) { \
    _type_##Factory::get()->listEntries(new vector<string>(), new vector<string>()); \
    return 0; \
  }
  FOR_EACH_MODULE(LIST_MODULES);

  // register own keys
  ValueParser vp(argv[1], "Main");
  KeyRegistry::instance()->fname = argv[1];
  _main = new MainModule;

  // set up pipe
  char *pipename = new char[MAX_STR_LEN];
  if (vp.get("pipe", pipename, MAX_STR_LEN) > 0)
    _pipe.init(pipename);

  // create modules
#define CREATE_MODULE(_type_) \
  char* _type_##Name = new char[MAX_STR_LEN]; \
  if (vp.get(_type_##Section.c_str(), _type_##Name, MAX_STR_LEN) < 1) { \
     WOUT("No " << _type_##Section.c_str() << " module specified, using dummy instead."); \
     _type_##Name = (char*)#_type_; \
  } \
  _##_type_ = _type_##Factory::get()->create(_type_##Name); \
  if (_##_type_==NULL) { \
    EOUT("Could not create " << _type_##Section.c_str() << " module of type \"" << _type_##Name << "\"."); \
    return -1; \
  }
  FOR_EACH_MODULE(CREATE_MODULE);

  // initialize mainloop
  int size[2] = {640, 480};
  vp.get("format", size, 2);
  vp.get("flip", _flip, false);
  vp.get("normalize", _norm, false);

  // initialize modules
  unsigned int add, total = 0;
#define INIT_MODULE(_type_) \
  add = 0; \
  if (!_##_type_->init(size[0], size[1], total, add, argv[1], #_type_)) { \
    EOUT("Init of module " << #_type_ << " failed!"); \
    return -1; \
  } \
  total += add;
  ///< \todo should check if cam returns image!
  FOR_EACH_MODULE(INIT_MODULE);
  if (total < 1) {
    EOUT("No image to process.");
    return -1;
  }
  if (total > MAX_IMAGES) {
    EOUT("Number of images (" << total << ") exceeds cache size (" << MAX_IMAGES << ")");
    return -1;
  }
  IOUT("Total number of available images: " << total);

  // prepare keyboard input
  set_conio_terminal_mode();
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT,  &sighandler);

  // main program loop
  _ImageSource->get_next_image(_images, _time);
  IplImage *tmp = cvCreateImage(cvSize(size[0], size[1]), IPL_DEPTH_8U, 3);
  char *cmd = new char[3];
  char *params = new char[MAX_STR_LEN];
  long int key;
  int ret, i;
  while (!_main->quit) {

    // event handling
    cmd[0] = cmd[1] = cmd[2] = 0;
    strcpy(params, "\0");
    ret = read(_fds, cmd, 3);
    if (ret > 0 || _Display->get_event(cmd, params) || get_cmd_from_pipe(cmd[0], params)) {
      key = cmd[0] + 256 * cmd[1] + 65536 * cmd[2];
      if (key != '*')
        KeyRegistry::instance()->delegate(key, params);
      else for (i=0; i<(int)strlen(params); i++)
        KeyRegistry::instance()->delegate(params[i], "");
    }

    // get image and pass through modules
    if (_ImageSource->get_next_image(_images, _time) && _images[0] != NULL) {
      if (_flip) { ///< \todo redo: flip ALL images from source
        cvConvertImage(_images[0], tmp, CV_CVTIMG_FLIP);
        _images[0] = tmp;
      }
      gettimeofday(&_time, NULL);
      _objects.clear();
      _Detector->detect(_images, _objects);
      _Tracker->select(_images, _objects);
      if (_norm) for (i=0; i<(int)_objects.size(); i++) { ///< \todo might be better to reverse approach: interval by default, pos only on demand
        _objects[i].x /= (size[0] / 2.);
        _objects[i].y /= (size[1] / 2.);
        _objects[i].x -= 1.;
        _objects[i].y -= 1.;
      }
      _Output->send(_objects, _time);
    }
    _Display->display(_images);
  }

  // deinitialize
#define DEINIT_AND_DELETE(_type_) \
  _##_type_->deinit(); \
  delete _##_type_;
  FOR_EACH_MODULE(DEINIT_AND_DELETE);
  reset_terminal_mode();
}

