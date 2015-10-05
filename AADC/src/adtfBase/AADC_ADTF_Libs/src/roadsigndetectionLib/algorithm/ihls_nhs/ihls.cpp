/**********************************************************************
* Copyright (c) 2014 BFFT Gesellschaft fuer Fahrzeugtechnik mbH.
* All rights reserved.
**********************************************************************
* $Author:: forchhe#$  $Date:: 2014-09-11 10:10:54#$ $Rev:: 25921   $
**********************************************************************/
#include "stdafx.h"
#include "roadsignDetectionHeaders.h"


tFloat32
retrieve_theta(tUInt r, tUInt g, tUInt b);

/**
 * It calculates theta bases on the equation provided in Valentine thesis.
 *
 * The returned theta is radian.
 */
tFloat32
retrieve_theta(tUInt r, tUInt g, tUInt b)
{
  tFloat32 theta;

  // The numerator part of equation
  tFloat32 numerator = r - (g * 0.5f) - (b * 0.5f);

  // The denominator part of equation
  tFloat32 denominator = static_cast<tFloat32> ((r * r) + (g * g) + (b * b) - (r * g) - (r * b) - (g * b));

  tFloat32 temp = numerator / sqrtf(denominator);
  theta = acos(temp);

  return theta;
}

/**
 * Calculating the hue value based on the blow formula:
 *
 * H = θ if B <= G
 * H = 2 * pi − θ if B > G
 *
 * The return value is normalised between 0 to 255.
 */
tFloat32
retrieve_normalised_hue(tUInt r, tUInt g, tUInt b)
{
  tFloat32 hue;
  if (b <= g)
    {
      hue = retrieve_theta(r, g, b);
    }
  else
    {
      hue = (2 * M_PI) - retrieve_theta(r, g, b);
    }

  return hue * 255 / (2 * M_PI);
}

/**
 * Luminance is calculated as:
 *
 * L = 0.210R + 0.715G + 0.072B
 */
tFloat32
retrieve_luminance(tUInt r, tUInt g, tUInt b)
{
  return (0.210f * r) + (0.715f * g) + (0.072f * b);
}

/**
 * Saturation is calculates as below:
 *
 * S = max(R, G, B) − min(R, G, B)
 */
tFloat32
retrieve_saturation(tUInt r, tUInt g, tUInt b)
{
  tFloat32 saturation;
  tUInt max = get_maximum(r, g, b);
  tUInt min = get_minimum(r, g, b);

  saturation = static_cast<tFloat32> (max - min);

  return saturation;
}

Mat
convert_rgb_to_ihls(Mat rgb_image)
{
  assert(rgb_image.channels() == 3);

  Mat ihls_image(rgb_image.rows, rgb_image.cols, CV_8UC3);

  for (tInt i = 0; i < rgb_image.rows; ++i)
    {
      const uchar* rgb_data = rgb_image.ptr<uchar> (i);
      uchar* ihls_data = ihls_image.ptr<uchar> (i);

      for (tInt j = 0; j < rgb_image.cols; ++j)
        {
          tUInt b = *rgb_data++;
          tUInt g = *rgb_data++;
          tUInt r = *rgb_data++;
          *ihls_data++ = (uchar) retrieve_saturation(r, g, b);
          *ihls_data++ = (uchar) retrieve_luminance(r, g, b);
          *ihls_data++ = (uchar) retrieve_normalised_hue(r, g, b);
        }
    }

  return ihls_image;
}
