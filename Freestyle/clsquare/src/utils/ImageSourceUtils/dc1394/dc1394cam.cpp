#include "dc1394cam.h"
#include "global.h"
#include "valueparser.h"
#include <dc1394/control.h>
#include <sys/time.h>
#include <string>

#define CHKRESMODE(xxx,yyy,zzz) if (width==xxx && height==yyy) resolution = DC1394_VIDEO_MODE_##xxx##x##yyy##_##zzz;
#define CHKRESALL(zzz) CHKRESMODE(640,480,zzz) else CHKRESMODE(800,600,zzz) else CHKRESMODE(1024,768,zzz) else CHKRESMODE(1280,960,zzz) else CHKRESMODE(1600,1200,zzz) \
else { EOUT("Color mode \"" << tmp << "\" does not support a resolution of " << width << "x" << height); return false; }

Dc1394Cam::Dc1394Cam ()
{
}

bool Dc1394Cam::init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  CLS::ValueParser vp(fname, chapter!=0 ? chapter : "ImageSource");
  _width = width;
  _height = height;
  _img = cvCreateImage(cvSize(_width,_height), IPL_DEPTH_8U, 3);
  add = 1;

  // determine parameters

  dc1394video_mode_t resolution;
  char tmp[255];
  vp.get("color", tmp, 255);
  if (strcmp(tmp, "rgb") == 0 || strcmp(tmp, "RGB") == 0) {
    _bgr = false;
    CHKRESALL(RGB8);
  } else if (strcmp(tmp, "bgr") == 0 || strcmp(tmp, "BGR") == 0) {
    _bgr = true;
    CHKRESALL(RGB8);
  } else if (strcmp(tmp, "yuv422") == 0 || strcmp(tmp, "YUV422") == 0) {
    CHKRESMODE(320,240,YUV422)
    else CHKRESALL(YUV422)
  } else if (strcmp(tmp, "mono") == 0 || strcmp(tmp, "MONO") == 0) {
    CHKRESALL(MONO8)
  } else {
    EOUT("Unsupported color mode \"" << tmp << "\".");
    return false;
  }

  double frate;
  vp.get("framerate", frate, 15);
  dc1394framerate_t framerate;
  switch (int(frate*1000)) {
    case 1875:   framerate = DC1394_FRAMERATE_1_875; break;
    case 3705:   framerate = DC1394_FRAMERATE_3_75;  break;
    case 7500:   framerate = DC1394_FRAMERATE_7_5;   break;
    case 15000:  framerate = DC1394_FRAMERATE_15;    break;
    case 30000:  framerate = DC1394_FRAMERATE_30;    break;
    case 60000:  framerate = DC1394_FRAMERATE_60;    break;
    case 120000: framerate = DC1394_FRAMERATE_120;   break;
    case 240000: framerate = DC1394_FRAMERATE_240;   break;
    default:
      EOUT("Framerate of " << frate << " fps not supported. Must be 1.875 * 2^x.");
      return false;
  }

  bool legacy;
  vp.get("legacy_mode", legacy, false);
  dc1394operation_mode_t mode = legacy ? DC1394_OPERATION_MODE_LEGACY : DC1394_OPERATION_MODE_1394B;

  long int guid;
  vp.get("guid", guid, -1);

  int channel;
  vp.get("memory_channel", channel, -1);

  // connect to cam

  _frame = 0;

  int tries = 0;
  bool again = false;
  _context = dc1394_new();

  do {

    if (dc1394_camera_enumerate(_context, &_cameras) != DC1394_SUCCESS) {
      EOUT("Failed enumerating cameras.");
      return false;
    }

    if (_cameras->num < 1) {
      EOUT("No cameras found.");
      return false;
    }

    _camera = dc1394_camera_new(_context, guid == -1 ? _cameras->ids[0].guid : guid);
    if (!_camera) {
      EOUT("Failed to initialize camera with guid " << guid);
      return false;
    }

    if (dc1394_feature_get_all(_camera, &_features) != DC1394_SUCCESS) {
      WOUT(0, "Unable to get feature set.");
    }

    if (channel > -1 && dc1394_memory_load(_camera, (unsigned int)channel) != DC1394_SUCCESS) {
      WOUT(0, "Could not read memory channel " << channel << ".");
    }

    // set parameters

    if (dc1394_video_set_operation_mode(_camera, mode) != DC1394_SUCCESS) {
      WOUT(0, "Unable to set operation mode.");
    }

    dc1394speed_t speed;
    if (dc1394_video_get_iso_speed(_camera, &speed) != DC1394_SUCCESS) {
      EOUT("Unable to get the ISO speed.");
      return false;
    }

    dc1394_video_set_iso_speed(_camera, mode == DC1394_OPERATION_MODE_LEGACY ? DC1394_ISO_SPEED_400 : DC1394_ISO_SPEED_800);
    if (dc1394_video_set_mode(_camera, resolution) != DC1394_SUCCESS) {
      EOUT("Could not set video mode.");
      return false;
    }

    dc1394_video_set_framerate(_camera, framerate);

    dc1394switch_t pwr;
    if (dc1394_video_get_transmission(_camera, &pwr) == DC1394_FAILURE) {
      EOUT("Can't querry ISO status.");
      return false;
    } else if (pwr == DC1394_ON ) {
      IOUT("Set ISO transmission off.");
      dc1394_video_set_transmission(_camera, DC1394_OFF);
    }

    if (dc1394_capture_setup(_camera, 5, DC1394_CAPTURE_FLAGS_DEFAULT) != DC1394_SUCCESS) {
      if (tries >= 2) {
        EOUT("Setup capture failed for a second time after a bus reset.");
        return false;
      }
      if (dc1394_reset_bus(_camera) != DC1394_SUCCESS) {
        EOUT("Setup capture failed, bus reset failed. Exit.");
        return false;
      }
      EOUT("Setup capture failed, did a bus reset and will try again.");
      again = true;
    } else again = false;
    tries++;
  } while (again);

  if (dc1394_video_set_transmission(_camera,DC1394_ON) !=DC1394_SUCCESS) {
    EOUT("unable to start camera iso transmission");
    return false;
  }

  if (dc1394_capture_dequeue(_camera, DC1394_CAPTURE_POLICY_WAIT, &_frame) != DC1394_SUCCESS) {
    EOUT("Unable to capture a frame.");
    return false;
  }

  dc1394_capture_enqueue(_camera, _frame);
  return true;
}

void Dc1394Cam::deinit ()
{
  if (_camera) {
    dc1394_video_set_transmission(_camera, DC1394_OFF);
    dc1394_capture_stop(_camera);
    dc1394_camera_free(_camera);
    _camera = 0;
  }
  if (_cameras) {
    dc1394_camera_free_list(_cameras);
    _cameras = 0;
  }
  cvReleaseImage(&_img);
}

Dc1394Cam::~Dc1394Cam()
{
  deinit();
  if (_context) dc1394_free(_context);
}

bool Dc1394Cam::get_next_image (IplImage** imgs, timeval& time)
{
  if (dc1394_capture_dequeue(_camera, DC1394_CAPTURE_POLICY_WAIT, &_frame) != DC1394_SUCCESS) {
    EOUT("Unable to capture a frame.");
    return false;
  }

  gettimeofday(&time, NULL);
  imgs[0] = _img;

  if (_frame->color_coding == DC1394_COLOR_CODING_YUV422) {
    int u,y0,v,y1, r,g,b;
    char* rgbbuffer = reinterpret_cast<char*>(_img->imageData);
    char* yuvbuffer = reinterpret_cast<char*>(_frame->image);
    for (int i = _width*_height; i > 0; i-=2, ++yuvbuffer, ++rgbbuffer) {
      u  = (unsigned char) *yuvbuffer - 128;
      y0 = (unsigned char) *(++yuvbuffer);
      v  = (unsigned char) *(++yuvbuffer) - 128;
      y1 = (unsigned char) *(++yuvbuffer);
      YUV2RGB (y0, u, v, r, g, b) ;
      *(rgbbuffer)   = b;
      *(++rgbbuffer) = g;
      *(++rgbbuffer) = r;
      YUV2RGB (y1, u, v, r, g, b) ;
      *(++rgbbuffer) = b;
      *(++rgbbuffer) = g;
      *(++rgbbuffer) = r;
    }
  }

  else if (_frame->color_coding == DC1394_COLOR_CODING_RGB8) {
    if (!_bgr) for (int i=0; i<_width*_height*3; i+=3) {
      _img->imageData[i]   = _frame->image[i+2];
      _img->imageData[i+1] = _frame->image[i+1];
      _img->imageData[i+2] = _frame->image[i];
    } else for (int i=0; i<_width*_height*3; i+=3) {
      _img->imageData[i]   = _frame->image[i];
      _img->imageData[i+1] = _frame->image[i+1];
      _img->imageData[i+2] = _frame->image[i+2];
    }
  }

  else if (_frame->color_coding == DC1394_COLOR_CODING_MONO8) {
    unsigned char* b = reinterpret_cast<unsigned char*>(_img->imageData);
    unsigned char* f = reinterpret_cast<unsigned char*>(_frame->image);
    for (int i = 0; i < _width*_height; i++, ++b, ++f) {
      *b = *f;
      *(++b) = *f;
      *(++b) = *f;
    }
  }
  _img->imageSize = _frame->image_bytes;

  dc1394_capture_enqueue(_camera, _frame);
  return true;
}

// interface to dc1394 features
/*
unsigned int Dc1394Cam::get_feature_value (dc1394feature_t feature) throw ()
  if (dc1394_feature_get_value(_camera, feature, &value) != DC1394_SUCCESS) {
bool Dc1394Cam::set_feature_value (dc1394feature_t feature, unsigned int value) throw ()
  if (dc1394_feature_set_value(_camera, feature, value) != DC1394_SUCCESS) {
bool Dc1394Cam::set_feature_mode (dc1394feature_t feature, dc1394feature_mode_t mode) throw ()
  if (dc1394_feature_set_mode(_camera, feature, mode) != DC1394_SUCCESS) {
dc1394feature_mode_t Dc1394Cam::get_feature_mode (dc1394feature_t feature) throw ()
  if (dc1394_feature_get_mode(camera, feature, &mode) != DC1394_SUCCESS) {
bool Dc1394Cam::is_feature_present (dc1394feature_t feature) throw ()
  if (dc1394_feature_is_present(_camera, feature, &present) != DC1394_SUCCESS) {
dc1394feature_modes_t Dc1394Cam::available_feature_modes (dc1394feature_t feature) throw ()
  if (dc1394_feature_get_modes(_camera, feature, &modes) != DC1394_SUCCESS) {
bool Dc1394Cam::get_feature_min_max_value (unsigned int* min, unsigned int* max, dc1394feature_t feature) throw ()
  if (dc1394_feature_get_boundaries(_camera, feature, min, max) != DC1394_SUCCESS) {
bool Dc1394Cam::set_white_balance (unsigned int u, unsigned int v) throw ()
  if (dc1394_feature_whitebalance_set_value(camera, u, v) != DC1394_SUCCESS) {
bool Dc1394Cam::get_white_balance (unsigned int* u, unsigned int* v) throw ()
  if (dc1394_feature_whitebalance_get_value(camera, u, v) != DC1394_SUCCESS) {
*/

//REGISTER(ImageSource, Dc1394Cam, "DC1394 camera");

