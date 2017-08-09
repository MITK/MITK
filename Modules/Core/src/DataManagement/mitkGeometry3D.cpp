/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
