/**********************************************************************
* Copyright (c) 2014 BFFT Gesellschaft fuer Fahrzeugtechnik mbH.
* All rights reserved.
**********************************************************************
* $Author:: spiesra $  $Date:: 2014-06-17 10:21:59#$ $Rev:: 23421   $
**********************************************************************/
/*
Description:    This is the header file for its corresponding cpp file. Provide the class for the module.

Date Created:   22/11/2011
Date Modified:  27/11/2011
                (Transformation from C style to C++ style. Use of vector instead of structure)

Author:         Team T
*/

#ifndef RotationalOffset_H
#define RotationalOffset_H

#include "../../stdafx.h"

using namespace std;

#define cPI 3.141
#define cMAX_POINTS 10



//Interface class

class IRO
{
public:
    typedef pair<int,int> Point2D;
    typedef vector<Point2D> Contour;

    virtual vector<float>  GetMinRadius( const Contour &contour ) = 0;
};


//RO module class

class cRotationalOffset : public IRO
{
private:
    //hold the <theta,radius> pair in vector
    vector<pair<float, float> > mThetaRad;
    //unsigned int              mNoOfPoints;
    vector<float>   mpOutput;

public:
    void            CartisanToPolar( const Contour &contour );
    void            FindMinimum( void );
    vector<float>   GetMinRadius( const Contour &contour );

    cRotationalOffset();
    ~cRotationalOffset();
};

#endif
