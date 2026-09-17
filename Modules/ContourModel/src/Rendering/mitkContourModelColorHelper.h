/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelColorHelper_h
#define mitkContourModelColorHelper_h

#include <mitkBaseRenderer.h>
#include <mitkColorProperty.h>
#include <mitkDataNode.h>

namespace mitk
{
  /** \brief Resolve the color a contour is rendered in.
   *
   * "contour.color" is the legacy spelling of the generic "color" property and still
   * takes precedence over it, so that scenes written before the switch keep the color
   * they were saved with. New code sets "color".
   */
  inline Color GetContourColor(const DataNode *node, const BaseRenderer *renderer)
  {
    float rgb[3] = { 0.9f, 1.0f, 0.1f };

    node->GetColor(rgb, renderer, "color");
    node->GetColor(rgb, renderer, "contour.color");

    Color color;
    color.SetRed(rgb[0]);
    color.SetGreen(rgb[1]);
    color.SetBlue(rgb[2]);

    return color;
  }
}

#endif
