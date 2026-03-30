/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNumericTypes_h
#define mitkNumericTypes_h

/**
 * \file mitkNumericTypes.h
 * \brief Convenience header that includes all MITK numeric types.
 *
 * Includes Vector, Point, Matrix, AffineTransform3D, Quaternion, Equal,
 * NumericConstants, and the deprecated VectorDeprecated conversions.
 */

#include <mitkAffineTransform3D.h>
#include <mitkEqual.h>
#include <mitkMatrix.h>
#include <mitkNumericConstants.h>
#include <mitkPoint.h>
#include <mitkQuaternion.h>
#include <mitkVector.h>

// This include holds the old deprecated ways to convert from itk to vtk and the likes.
// Calls to these functions shall be removed in future bug squashings so that this include can be removed.
#include <mitkVectorDeprecated.h>

#endif
