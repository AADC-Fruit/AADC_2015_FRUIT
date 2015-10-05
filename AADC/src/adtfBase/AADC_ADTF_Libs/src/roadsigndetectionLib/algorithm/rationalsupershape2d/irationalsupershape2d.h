#ifndef IRATIONALSUPERSHAPE2D_H
#define IRATIONALSUPERSHAPE2D_H

/*
*   Implemented By GROUP U
*   - Ajad Chhatkuli
*   - Andru Putra Twinanda
*   - Angga Reza Fardana
*   - Lijia Gao
*   - Ozan Tonkal
*   - Yukti Suri
*
*   This class is used to optimize the contour points
*   - to fit it into a gielis curve.
*
*   In order to run the algorithm, Run method is called.
*   Input:
*   - vector of points representing the contour
*   - a boolean to determine if it should be normalized
*   - an integer to determine which implicit function
*   - - should be used.
*   Output:
*   - vector of optimized points
*/

#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/StdVector>

using namespace std;
using namespace Eigen;

// In order to be able to use vector2d
// -in stl containers, this line is mandatory.
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Vector2d)

class IRationalSuperShape2D
{
    public:
    IRationalSuperShape2D() {}

    virtual vector< Vector2d > Run(const vector< Vector2d > &data,
                                   const vector< float > &rotOffsets,
                                   bool normalization = true,
                                   int functionUsed = 1) = 0;
};

#endif // IRATIONALSUPERSHAPE2D_H
