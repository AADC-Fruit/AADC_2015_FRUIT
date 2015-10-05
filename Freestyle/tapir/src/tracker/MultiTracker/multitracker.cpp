#include "multitracker.h"
#include <cmath>
#include <cstring>

#define SSSET(xxx) { ss.str(""); ss << xxx; };

bool MultiTracker::init (const int width, const int height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "Tracker");
  stringstream ss;

  unsigned int tmpadd = 0;
  add = 0;
  for (_i=0; ; _i++) {

    // try to get next tracker name
    int len, space = -1;
    char *name = new char[255];
    SSSET("tracker_" << _i);
    len = vp.get(ss.str().c_str(), name, 255);
    if (len < 0) break;

    // if there's a space, treat next segment as chapter
    for (int _k=0; _k<len; _k++)
      if (name[_k] == ' ') {
        space = _k;
        name[_k] = '\0';
    }

    // create tracker
    Tracker *base = TrackerFactory::get()->create(name);
    if (base == NULL) {
      EOUT("MultiTracker could not create a module of type " << name << "!");
      return false;
    }

    // determine config chapter
    char *chapter;
    SSSET("Tracker_" << _i);
    if (space < 0) {
      chapter = new char[ss.str().size() + 1];
      std::strcpy(chapter, ss.str().c_str());
    } else {
      chapter = &name[space+1];
    }

    // try to initialize tracker
    if (!base->init(width, height, imgs+add, tmpadd, fname, chapter)) {
      EOUT("MultiTracker failed to initialize tracker " << name << " !");
      return false;
    }
    add += tmpadd;

    _slaves.push_back(base);
  }

  return true;
}

void MultiTracker::select (IplImage** imgs, std::vector<TapirObject>& objects)
{
  for (_i=0; _i<(int)_slaves.size(); _i++)
    _slaves.at(_i)->select(imgs, objects);
}

void MultiTracker::deinit ()
{
  for (_i=0; _i<(int)_slaves.size(); _i++)
    _slaves.at(_i)->deinit();
}

void MultiTracker::notify (const long int cmd, const char* params)
{
  for (_i=0; _i<(int)_slaves.size(); _i++)
    _slaves.at(_i)->notify(cmd, params);
}

REGISTER(Tracker, MultiTracker, "Meta-tracker for using several modules at once.");
