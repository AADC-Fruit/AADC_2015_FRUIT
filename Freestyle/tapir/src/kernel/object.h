#ifndef _TAPIR_OBJECT_H_
#define _TAPIR_OBJECT_H_

/** Represents a detected object. */
class TapirObject
{
 public:
  TapirObject (double x=-1, double y=-1, double size=-1, double angle=0, double confidence=100)
  {
    this->x = x;
    this->y = y;
    this->z = 0.;
    this->size = size;
    this->angle = angle;
    this->confidence = confidence;
  };
  ~TapirObject () {};
  double x, y, z, size, angle, confidence;
};

#endif
