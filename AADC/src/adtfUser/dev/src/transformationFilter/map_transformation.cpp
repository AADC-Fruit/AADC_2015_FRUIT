#include "map_transformation.h"
#include <iostream>
#include <fstream>
 
// -------------------------------------------------------------------------------------------------
void MapTransformation::transform(Vector2** transformation_matrix, float turn_x, float turn_y, double rotation, size_t height, size_t width, size_t cell_size) {
// -------------------------------------------------------------------------------------------------
  // Calculate offsets and transformation angle
  bool rotate_flag = (fabs(rotation) < 0.001) ? false : true;
  //std::cout << "Rotation: " << rotation*180.0/M_PI << std::endl; 
  // Transform the old map
  for (size_t i = 0; i < height; i++) {
    for (size_t j = 0; j < width; j++) {
      if(rotate_flag) {
        float new_x = transform_x(turn_x, turn_y, j * cell_size, i * cell_size, -rotation);
        float new_y = transform_y(turn_x, turn_y, j * cell_size, i * cell_size, -rotation);
        int transformed_x = round(new_x / (float)(cell_size));
        int transformed_y = round(new_y / (float)(cell_size));
        //std::cout << "Rotation: " << i << ", " << j << " -> " << transformed_y << ", " << transformed_x << std::endl; 
        if(check_boundaries(transformed_x, transformed_y, width, height)) {
          //std::cout << "[MT] Set (" << i << ", " << j << ")" << std::endl;
          transformation_matrix[i][j] = Vector2(transformed_x, transformed_y);
        } 
      } else {
        int transformed_x = j;
        int transformed_y = round(i + turn_y / (float)(cell_size));
        //std::cout << "Translation: " << i << ", " << j << " -> " << transformed_y << ", " << transformed_x << std::endl; 
        if(check_boundaries(transformed_x, transformed_y, width, height)) {
          //std::cout << "[MT] Set (" << i << ", " << j << ")" << std::endl;
          transformation_matrix[i][j] = Vector2(transformed_x, transformed_y);
        } 
      }
    }
  }
}

// -------------------------------------------------------------------------------------------------
bool MapTransformation::check_boundaries(int x, int y, int boundary_x, int boundary_y) {
// -------------------------------------------------------------------------------------------------
  if(x < 0 || x >= boundary_x) return false;
  else if(y < 0 || y >= boundary_y) return false;
  else return true;
}

// -------------------------------------------------------------------------------------------------
double MapTransformation::transform_x(float turn_x, float turn_y, int value_x, int value_y, double rotation) {
// -------------------------------------------------------------------------------------------------
  return (turn_x + (cos(rotation) * ((float)(value_x) - turn_x)) - (sin(rotation) * ((float)(value_y) - turn_y)));
}

// -------------------------------------------------------------------------------------------------
double MapTransformation::transform_y(float turn_x, float turn_y, int value_x, int value_y, double rotation) {
// -------------------------------------------------------------------------------------------------
  return (turn_y + (sin(rotation) * ((float)(value_x) - turn_x)) + (cos(rotation) * ((float)(value_y) - turn_y)));
}
