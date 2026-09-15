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

#include <vector>

class vtkColorTransferFunction;

namespace mitk
{
  /**
   * \brief Window a color transfer function by a level shift and a width.
   *
   * Samples the color at \p samples evenly spaced intensities over
   * [dataMin, dataMax], returned as that many r, g, b triples - 3 * \p samples
   * doubles - or as nothing where the arguments describe no window. The window
   * is centered on \p source's own range; \p shift offsets the center (level)
   * and \p width sizes it. Each triple samples \p source at the intensity the
   * window maps it back to, so shift 0 with width == the source range
   * reproduces \p source.
   *
   * The even spacing is a contract rather than an implementation detail: it is
   * what allows the result to be applied with
   * vtkColorTransferFunction::BuildFunctionFromTable(dataMin, dataMax, samples,
   * data()), which sorts the function once for the whole table where adding the
   * nodes one at a time re-sorts it on every insert.
   */
  MITKVOLUMEVISUALIZATION_EXPORT std::vector<double> ResampleColorWindow(
    vtkColorTransferFunction *source,
    double dataMin,
    double dataMax,
    double shift,
    double width,
    int samples = 256);
}

#endif
