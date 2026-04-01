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
  /** \brief Conversion routines between different color spaces. */
  namespace ColorConversions
  {
    /**
     * \brief Convert an HSV color to RGB.
     *
     * \param h Hue component (0 to 360).
     * \param s Saturation component (0 to 1).
     * \param v Value component (0 to 1).
     * \param[out] r Red component (0 to 1).
     * \param[out] g Green component (0 to 1).
     * \param[out] b Blue component (0 to 1).
     */
    void Hsv2Rgb(float h, float s, float v, float &r, float &g, float &b);

    /**
     * \brief Convert an RGB color to HSV.
     *
     * \param r Red component (0 to 1).
     * \param g Green component (0 to 1).
     * \param b Blue component (0 to 1).
     * \param[out] h Hue component (0 to 360).
     * \param[out] s Saturation component (0 to 1).
     * \param[out] v Value component (0 to 1).
     */
    void Rgb2Hsv(float r, float g, float b, float &h, float &s, float &v);

  }
}

#endif
