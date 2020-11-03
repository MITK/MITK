/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iomanip>
#include <sstream>

#include "mitkGeometry3D.h"

#include "mitkApplyTransformMatrixOperation.h"
#include "mitkInteractionConst.h"
#include "mitkMatrixConvert.h"
#include "mitkPointOperation.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkRotationOperation.h"
#include <vtkMatrix4x4.h>
#include <vtkMatrixToLinearTransform.h>

// Standard constructor for the New() macro. Sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D() : BaseGeometry()
{
}
mitk::Geometry3D::Geometry3D(const Geometry3D &other) : BaseGeometry(other)
{
}

mitk::Geometry3D::~Geometry3D()
{
}

itk::LightObject::Pointer mitk::Geometry3D::InternalClone() const
{
  Self::Pointer newGeometry = new Self(*this);
  newGeometry->UnRegister();
  return newGeometry.GetPointer();
}
