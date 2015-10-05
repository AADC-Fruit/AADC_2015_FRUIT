/**********************************************************************
* Copyright (c) 2014 BFFT Gesellschaft fuer Fahrzeugtechnik mbH.
* All rights reserved.
**********************************************************************
* $Author:: forchhe#$  $Date:: 2014-09-11 10:10:54#$ $Rev:: 25921   $
**********************************************************************/
/*
 * ihls.h
 *
 * Header file for IHLS (Improved Hue Luminance Saturation)
 *
 * The main function is convert_rgb_to_ihls which must be called
 * from outside. The rest of functions are provided publicly if
 * someone wants to directly retrieve hue, saturation or luminance
 * based on RGB values.
 *
 */

#ifndef IHLS_H_
#define IHLS_H_


/**
 * Returning hue value of RGB.
 */
inline tFloat32
retrieve_normalised_hue(tUInt r, tUInt g, tUInt b);

/**
 * Returning luminance value of RGB.
 */
inline tFloat32
retrieve_luminance(tUInt r, tUInt g, tUInt b);

/**
 * Returning saturation value of RGB.
 */
inline tFloat32
retrieve_saturation(tUInt r, tUInt g, tUInt b);

/**
 * This function converts the an RGB image to IHLS and returns it.
 *
 * The reason that we have a return matrix and not overriding the original
 * RGB image, is to keep the original RGB values for reference.
 *
 */
Mat
convert_rgb_to_ihls(Mat rgb_image);

#endif /* IHLS_H_ */
