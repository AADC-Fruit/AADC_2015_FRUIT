/**********************************************************************
* Copyright (c) 2014 BFFT Gesellschaft fuer Fahrzeugtechnik mbH.
* All rights reserved.
**********************************************************************
* $Author:: forchhe#$  $Date:: 2014-09-11 10:10:54#$ $Rev:: 25921   $
**********************************************************************/
/*
 * math_utils.h
 *
 * Some common mathematical functions, like getting minimum and maximum of
 * three integers.
 *
 * TODO: add more function
 * TODO: think more generic
 */
#ifndef MATH_UTILS_H_
#define MATH_UTILS_H_

/**
 * Return the greatest value of three passed arguments.
 *
 * Unsigned integer has been used, because RGB values cant be minus.
 */
tUInt
get_maximum(tUInt r, tUInt g, tUInt b);

/**
 * Return the lowest value of three passed arguments.
 *
 * Unsigned integer has been used, because RGB values cant be minus.
 */
tUInt
get_minimum(tUInt r, tUInt g, tUInt b);

#endif /* MATH_UTILS_H_ */
