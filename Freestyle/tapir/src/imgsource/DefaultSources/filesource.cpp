#include "filesource.h"
#include "global.h"
#include <fstream>
#include <sstream>
#include <dirent.h>

bool FileImageSource::init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "ImageSource");
  char filename[255];
  if (vp.get("path", filename, 255) < 1) {
    EOUT("No path to image files specified.");
    return false;
  }
  IOUT("Reading files from path \"" << filename << "\".");


  DIR *directory;
  if (!(directory=opendir(filename))) {
    EOUT("Could not open directory.");
    return false;
  }

  struct dirent *entry;
  while ((entry=readdir(directory))) {
    if (strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0)
      continue;
    _filelist.push_back(filename+std::string("/")+std::string(entry->d_name));
  }
  closedir(directory);

  if (_filelist.size() < 1) {
    EOUT("No files found.");
    return false;
  }

  _current = 0;
  _img = 0;
  add = 1;
  return true;
}

void FileImageSource::deinit ()
{
  cvReleaseImage(&_img);
}

bool FileImageSource::get_next_image (IplImage** imgs, timeval& time)
{
  if (_img != 0)
    cvReleaseImage(&_img);
  _img = cvLoadImage(_filelist[_current].c_str(), CV_LOAD_IMAGE_COLOR);
  if (_img == 0) {
    EOUT("Cannot load file " << _filelist[_current].c_str() << "!");
    return false;
  }
  _current++;
  if (_current >= _filelist.size())
    _current = 0;

  imgs[0] = _img;
  return true;
}

REGISTER(ImageSource, FileImageSource, "Reads images from file.");
