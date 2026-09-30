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

  /**
   * \brief Window a color transfer function in place, by moving the nodes it
   *        already has rather than by sampling new ones.
   *
   * The same window as ResampleColorWindow, over the same range, and the same
   * colors - carried by the function's own handful of nodes instead of 256
   * evenly spaced samples. That is what makes the result editable by hand: one
   * node per color the function names, where the table has one per pixel column
   * and no way to tell which of them was meant.
   *
   * The window is read from \p function's own range, before anything moves, so
   * windowing a function twice windows it twice over.
   *
   * Nodes are left no closer than (dataMax - dataMin) / 255 - the spacing the
   * resampled table guarantees - because VTK sizes its lookup table by the
   * smallest gap between nodes, and a window narrow enough to collapse them
   * would ask the renderer for a table of millions of entries.
   */
  MITKVOLUMEVISUALIZATION_EXPORT void ApplyColorWindow(
    vtkColorTransferFunction *function,
    double dataMin,
    double dataMax,
    double shift,
    double width);
}

#endif
