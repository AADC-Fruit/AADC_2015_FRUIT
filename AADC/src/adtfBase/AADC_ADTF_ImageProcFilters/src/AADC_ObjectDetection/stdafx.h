#ifndef __STD_INCLUDES_HEADER
#define __STD_INCLUDES_HEADER

// ADTF header
#include <adtf_platform_inc.h>
#include <adtf_plugin_sdk.h>
using namespace adtf;

#include <adtf_graphics.h>
using namespace adtf_graphics;

// opencv header
#include <opencv/cv.h>
#include <opencv/highgui.h>
using namespace cv;
/*!
this struct holds the output mode
*/
typedef struct tag_OutputModestruct
{
    tInt nXRes;					/**< x resulotion*/
    tInt nYRes;					/**< y resulotion*/
    tInt nFPS;					/**< frames per second*/
} tOutputModestruct;

/*!
this struct holds the object struct
*/
typedef struct tag_ObjStruct
{
	tInt16 nObjNr;				/**< number of this object */
	tInt16 nObjXLPos;			/**< XL position of this object */
	tInt16 nObjXLPixel;			/**< XL pixel of this object */
	tInt16 nObjXLDepth;			/**< XL depth of this object */
	tInt16 nObjXMPos;			/**< XM position of this object */
	tInt16 nObjXMDepth;			/**< XM depth of this object */
	tInt16 nObjXRPos;			/**< XR position of this object */
	tInt16 nObjXRPixel;			/**< XR pixel of this object */
	tInt16 nObjXRDepth;			/**< XR depth of this object */
	tInt8  nGueltig;			/**< flag if valid*/
} tObjStruct;

/*!
this struct holds 3D Vector
*/
typedef struct tag_Vector3D
{
	tInt16 X;				/**< x coordinate*/
	tInt16 Y;				/**< y coordinate */
	tInt16 Z;				/**< z coordinate*/
} tVector3D;

/*!
this struct holds 3D Vector in floats
*/
typedef struct tag_FloatVector3D
{
	tFloat32 X;				/**< x coordinate*/
	tFloat32 Y;				/**< y coordinate*/
	tFloat32 Z;				/**< z coordinate*/
} tFloatVector3D;


#endif // __STD_INCLUDES_HEADER
