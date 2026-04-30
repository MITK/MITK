/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNumericConstants_h
#define mitkNumericConstants_h

#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Scalar type used throughout MITK for geometric computations. */
  typedef double ScalarType;

  /** \brief Epsilon value for floating point comparison (vnl_math::eps * 100). */
  MITKCORE_EXPORT extern const ScalarType eps;

  /** \brief Square root of epsilon (vnl_math::sqrteps). */
  MITKCORE_EXPORT extern const ScalarType sqrteps;

  /** \brief A large value (std::numeric_limits<ScalarType>::max()). */
  MITKCORE_EXPORT extern const double large;
}

#endif
