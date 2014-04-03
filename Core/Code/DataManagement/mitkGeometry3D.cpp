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

#include <sstream>
#include <iomanip>

#include "mitkGeometry3D.h"

#include "mitkRotationOperation.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkApplyTransformMatrixOperation.h"
#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>
#include "mitkMatrixConvert.h"

// Standard constructor for the New() macro. Sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D() :  BaseGeometry()
{
}
mitk::Geometry3D::Geometry3D(const Geometry3D& other) : BaseGeometry(other)
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
