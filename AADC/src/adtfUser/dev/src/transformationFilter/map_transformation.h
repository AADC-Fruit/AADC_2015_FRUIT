#ifndef _MAP_TRANSFORMATION_H_
#define _MAP_TRANSFORMATION_H_

#include "../util/vector2.h"
#include <stdlib.h>
#include <math.h>

class MapTransformation {
 public:
  void transform(Vector2** transformation_matrix, float turn_x, float turn_y, double rotation, size_t height, size_t width, size_t cell_size);
 
 private:
  bool check_boundaries(int x, int y, int boundary_x, int boundary_y);
  double last_gyro_value_;
  double transform_x(float turn_x, float turn_y, int value_x, int value_y, double rotation);
  double transform_y(float turn_x, float turn_y, int value_x, int value_y, double rotation);
};

#endif //_MAP_TRANSFORMATION_H_
