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


#include "mitkApplyTransformMatrixOperation.h"

namespace mitk
{

ApplyTransformMatrixOperation
::ApplyTransformMatrixOperation( OperationType operationType, vtkSmartPointer<vtkMatrix4x4> matrix, Point3D refPoint )
: Operation(operationType), m_vtkMatrix( matrix ), m_referencePoint( refPoint )
{
}

ApplyTransformMatrixOperation
::~ApplyTransformMatrixOperation()
{
}

vtkSmartPointer<vtkMatrix4x4> ApplyTransformMatrixOperation::GetMatrix()
{
  return m_vtkMatrix;
}

Point3D ApplyTransformMatrixOperation::GetReferencePoint()
{
  return m_referencePoint;
}

} // namespace mitk
