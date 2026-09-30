/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTransferFunctionTransform.h"

#include <vtkColorTransferFunction.h>

#include <algorithm>

namespace mitk
{
  std::vector<double> ResampleColorWindow(vtkColorTransferFunction* source, double dataMin, double dataMax, double shift, double width, int samples)
  {
    std::vector<double> result;

    if (source == nullptr || samples < 2 || dataMax <= dataMin)
      return result;

    const double *sourceRange = source->GetRange();
    const double origMin = sourceRange[0];
    const double origSpan = std::max(1e-6, sourceRange[1] - sourceRange[0]);

    // Center the window on the source's own range; shift moves the center
    // (level), width sizes it
    const double level = 0.5 * (sourceRange[0] + sourceRange[1]) + shift;
    const double windowMin = level - 0.5 * width;
    const double windowSpan = std::max(1e-6, width); // guards width -> 0

    // Three per sample, and no intensity among them: evenly spaced positions are
    // implied by [dataMin, dataMax] and the count, so whoever applies the table
    // derives them rather than reading them back.
    result.resize(3 * static_cast<std::size_t>(samples));

    for (int i = 0; i < samples; ++i)
    {
      const double x = dataMin + (dataMax - dataMin) * i / (samples - 1);

      // Invert the window: which source intensity does this node read from?
      const double srcX = origMin + (x - windowMin) / windowSpan * origSpan;

      // Writes the three doubles in place; clamps to the endpoint color outside
      // source's range.
      source->GetColor(srcX, result.data() + 3 * static_cast<std::size_t>(i));
    }

    return result;
  }

  void ApplyColorWindow(vtkColorTransferFunction *function, double dataMin, double dataMax, double shift, double width)
  {
    if (function == nullptr || function->GetSize() == 0 || dataMax <= dataMin)
      return;

    const double origMin = function->GetRange()[0];
    const double origMax = function->GetRange()[1];
    const double origSpan = std::max(1e-6, origMax - origMin);

    const double level = 0.5 * (origMin + origMax) + shift;
    const double windowMin = level - 0.5 * width;
    const double windowSpan = std::max(1e-6, width); // guards width -> 0

    // Four per node - intensity, r, g, b. Copied out before anything is
    // dropped, since what follows computes where each of these lands.
    const int size = function->GetSize();
    std::vector<double> nodes(4 * static_cast<std::size_t>(size));
    std::copy_n(function->GetDataPointer(), nodes.size(), nodes.begin());

    const double minSpacing = (dataMax - dataMin) / 255.0;

    function->RemoveAllPoints();

    double previous = 0.0;

    for (int i = 0; i < size; ++i)
    {
      const double *node = nodes.data() + 4 * static_cast<std::size_t>(i);

      // Where the window puts a source intensity: the forward form of the
      // lookup ResampleColorWindow inverts once per sample.
      double x = windowMin + (node[0] - origMin) / origSpan * windowSpan;

      // The nodes came out sorted, so keeping each one clear of the last keeps
      // the whole function sorted as well as spaced.
      if (i > 0)
        x = std::max(x, previous + minSpacing);

      previous = x;

      function->AddRGBPoint(x, node[1], node[2], node[3]);
    }
  }
}
