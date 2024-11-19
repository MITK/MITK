/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAffineTransform3D_h
#define mitkAffineTransform3D_h

#include <mitkNumericConstants.h>
#include <itkScalableAffineTransform.h>
#include <nlohmann/json_fwd.hpp>
#include <MitkCoreExports.h>

namespace mitk
{
  using AffineTransform3D = itk::ScalableAffineTransform<ScalarType, 3>;

  /** \brief Write transform (4x4 matrix) as JSON array with 16 elements.
   */
  MITKCORE_EXPORT void ToJSON(nlohmann::json& j, AffineTransform3D::ConstPointer transform);

  /** \brief Read transform from JSON array (16 elements, resp. 4x4 matrix).
   */
  MITKCORE_EXPORT void FromJSON(const nlohmann::json& j, AffineTransform3D::Pointer transform);
}

#endif
