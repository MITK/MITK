/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransferFunctionTransform_h
#define mitkTransferFunctionTransform_h

#include <MitkVolumeVisualizationUIExports.h>

#include <mitkTransferFunction.h>

namespace mitk
{
  /**
   * \brief Remap the intensity axis of scalar-opacity control points.
   *
   * Applies x' = scale * x + offset to each point's position and leaves the
   * opacity values untouched. Shift is (scale 1, offset delta); width is a
   * scale about a center.
   */
  MITKVOLUMEVISUALIZATIONUI_EXPORT TransferFunction::ControlPoints RemapIntensity(
    const TransferFunction::ControlPoints &points, double scale, double offset);

  /**
   * \brief Remap the intensity axis of color control points.
   *
   * Applies x' = scale * x + offset to each point's position and leaves the
   * RGB values untouched.
   */
  MITKVOLUMEVISUALIZATIONUI_EXPORT TransferFunction::RGBControlPoints RemapIntensity(
    const TransferFunction::RGBControlPoints &points, double scale, double offset);
}

#endif
