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
   * \brief Window a color transfer function onto evenly spaced nodes.
   *
   * Rebuilds the color as \p samples evenly spaced nodes spanning
   * [dataMin, dataMax]. The node at intensity x is colored by sampling \p source
   * at the intensity the window [windowMin, windowMax] maps x back to, so the
   * window edges land on \p source's own range endpoints. Even node spacing keeps
   * VTK's volume color LUT a fixed, small size for any window width. \p source is
   * sampled with its own clamping and color space, so a neutral window
   * (windowMin/windowMax == \p source's range) reproduces \p source.
   */
  MITKVOLUMEVISUALIZATIONUI_EXPORT TransferFunction::RGBControlPoints ResampleColorWindow(
    vtkColorTransferFunction *source,
    double dataMin,
    double dataMax,
    double windowMin,
    double windowMax,
    int samples = 256);
}

#endif
