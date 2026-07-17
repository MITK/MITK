/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTransferFunctionTransform.h"

namespace mitk
{
  TransferFunction::ControlPoints RemapIntensity(
    const TransferFunction::ControlPoints &points, double scale, double offset)
  {
    TransferFunction::ControlPoints result;
    result.reserve(points.size());

    for (const auto &[x, value] : points)
    {
      result.emplace_back(scale * x + offset, value);
    }

    return result;
  }

  TransferFunction::RGBControlPoints RemapIntensity(
    const TransferFunction::RGBControlPoints &points, double scale, double offset)
  {
    TransferFunction::RGBControlPoints result;
    result.reserve(points.size());

    for (const auto &[x, color] : points)
    {
      result.emplace_back(scale * x + offset, color);
    }

    return result;
  }
}