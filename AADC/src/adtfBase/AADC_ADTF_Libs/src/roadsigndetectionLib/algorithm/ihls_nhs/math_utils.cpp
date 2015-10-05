/**********************************************************************
* Copyright (c) 2014 BFFT Gesellschaft fuer Fahrzeugtechnik mbH.
* All rights reserved.
**********************************************************************
* $Author:: forchhe#$  $Date:: 2014-09-11 10:10:54#$ $Rev:: 25921   $
**********************************************************************/
#include "stdafx.h"
#include "roadsignDetectionHeaders.h"

/**
 * Return the greatest value of three passed arguments.
 *
 * Unsigned integer has been used, because RGB values cant be minus.
 */
tUInt
get_maximum(tUInt r, tUInt g, tUInt b)
{
  if (r >= g)
    {
      if (r >= b)
        {
          return r;
        }
      else
        {
          return b;
        }
    }
  else
    {
      if (g >= b)
        {
          return g;
        }
      else
        {
          return b;
        }
    }
}

/**
 * Return the lowest value of three passed arguments.
 *
 * Unsigned integer has been used, because RGB values cant be minus.
 */
tUInt
get_minimum(tUInt r, tUInt g, tUInt b)
{
  if (r <= g)
    {
      if (r <= b)
        {
          return r;
        }
      else
        {
          return b;
        }
    }
  else
    {
      if (g <= b)
        {
          return g;
        }
      else
        {
          return b;
        }
    }
}
