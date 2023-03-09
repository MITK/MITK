/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkColorConversions_h
#define mitkColorConversions_h

namespace mitk
{
  /// some conversion routines to convert between different color spaces
  namespace ColorConversions
  {
    /// convert a HSV color to RGB color, H from 0 to 360, all other parameters 0 to 1
    void Hsv2Rgb(float h, float s, float v, float &r, float &g, float &b);
    /// convert a RGB color to HSV color, rgb parameters from 0 to 1
    void Rgb2Hsv(float r, float g, float b, float &h, float &s, float &v);

  }
}

#endif
