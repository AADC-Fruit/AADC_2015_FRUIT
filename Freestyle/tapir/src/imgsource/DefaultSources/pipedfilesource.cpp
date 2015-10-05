#include "pipedfilesource.h"
#include "global.h"

bool PipedFileImageSource::init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "ImageSource");
  vp.get("mask", _mask, MAX_STR_LEN);  

  char path[MAX_STR_LEN];
  vp.get("pipe", path, MAX_STR_LEN);
  if (!_pipe.init(path)) return false;

  vp.get("framerate", _rate, 30);
  _rate = 1000000 / _rate;

  _img = 0;
  add = 1;
  return true;
}

void PipedFileImageSource::deinit ()
{
  cvReleaseImage(&_img);
}

bool PipedFileImageSource::get_next_image (IplImage** imgs, timeval& time)
{
  imgs[0] = _img;
  usleep(_rate);

  // check pipe
  _bytes = _pipe.check();
  if (_bytes < 1) return true;

  // find newest token
  const char* message = _pipe.buffer();
  int last[2] = {-1, -1};

  for (_i=0; _i<_bytes; _i++) {
    if (message[_i] == '\n') {
      last[0] = last[1];
      last[1] = _i;
    }
  }
  if (last[1] == 0) {
    WOUT("No delimiter found in message.");
    return false;
  }

  // copy last token
  int len = last[1] - last[0] - 1;
  memcpy(_token, &message[last[0]+1], sizeof(char) * len);
  _token[len] = '\0';

  // open file
  sprintf(_buffer, _mask, _token);
  if (_img != 0)
    cvReleaseImage(&_img);
  _img = cvLoadImage(_buffer, CV_LOAD_IMAGE_COLOR);
  imgs[0] = _img;
  if (_img == 0) {
    EOUT("Cannot load file " << _buffer << "!");
    return false;
  }

  return true;
}

REGISTER(ImageSource, PipedFileImageSource, "Reads images from file as indicated by pipe.");
