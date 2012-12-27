/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

namespace mitk {

  /// some conversion routines to convert between different color spaces
  namespace ColorConversions {

    /// convert a HSV color to RGB color, H from 0 to 360, all other parameters 0 to 1
    void Hsv2Rgb(float h, float s, float v, float& r, float& g, float& b);
    /// convert a RGB color to HSV color, rgb parameters from 0 to 1
    void Rgb2Hsv(float r, float g, float b, float &h, float &s, float &v);

  } // ColorConversion

} // mitk

