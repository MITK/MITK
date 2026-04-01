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
  /**
   * \brief 3D affine transform type used throughout MITK.
   *
   * Alias for itk::ScalableAffineTransform with MITK's ScalarType and 3 dimensions.
   */
  using AffineTransform3D = itk::ScalableAffineTransform<ScalarType, 3>;

  /**
   * \brief Serialize an AffineTransform3D to a JSON array with 16 elements (row-major 4x4 matrix).
   *
   * \param[out] j The JSON value to write to.
   * \param[in] transform The transform to serialize.
   */
  MITKCORE_EXPORT void ToJSON(nlohmann::json& j, AffineTransform3D::ConstPointer transform);

  /**
   * \brief Deserialize an AffineTransform3D from a JSON array with 16 elements (row-major 4x4 matrix).
   *
   * \param[in] j The JSON value to read from.
   * \param[in,out] transform The transform to populate.
   */
  MITKCORE_EXPORT void FromJSON(const nlohmann::json& j, AffineTransform3D::Pointer transform);
}

#endif
