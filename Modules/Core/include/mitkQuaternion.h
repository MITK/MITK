/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkQuaternion.h
 * \brief Defines the mitk::Quaternion type alias for quaternion-based rotation representation.
 *
 * Provides a convenience typedef for \c vnl_quaternion\<ScalarType\> so that MITK code
 * can use \c mitk::Quaternion without depending on VNL headers directly.
 *
 * \ingroup Core
 */

#ifndef mitkQuaternion_h
#define mitkQuaternion_h

#include <mitkNumericConstants.h>
#include <vnl/vnl_quaternion.h>

namespace mitk
{
  /**
   * \brief Quaternion type for 3-D rotations, based on \c vnl_quaternion\<ScalarType\>.
   *
   * \sa mitk::ScalarType
   */
  typedef vnl_quaternion<ScalarType> Quaternion;
}

#endif
