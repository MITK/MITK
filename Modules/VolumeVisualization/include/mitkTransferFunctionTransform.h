/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransferFunctionTransform_h
#define mitkTransferFunctionTransform_h

#include <MitkVolumeVisualizationExports.h>

#include <mitkTransferFunction.h>

namespace mitk
{
  /**
   * \brief Window a color transfer function by a level shift and a width.
   *
   * Rebuilds the color as \p samples evenly spaced nodes over [dataMin, dataMax].
   * The window is centered on \p source's own range; \p shift offsets the center
   * (level) and \p width sizes it. Each node samples \p source at the intensity
   * the window maps it back to, so shift 0 with width == the source range
   * reproduces \p source.
   */
  MITKVOLUMEVISUALIZATION_EXPORT TransferFunction::RGBControlPoints ResampleColorWindow(
    vtkColorTransferFunction *source,
    double dataMin,
    double dataMax,
    double shift,
    double width,
    int samples = 256);
}

#endif
