/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkApplyTransformMatrixOperation.h"

namespace mitk
{
  ApplyTransformMatrixOperation::ApplyTransformMatrixOperation(OperationType operationType,
                                                               vtkSmartPointer<vtkMatrix4x4> matrix,
                                                               Point3D refPoint)
    : Operation(operationType), m_vtkMatrix(matrix), m_referencePoint(refPoint)
  {
  }

  ApplyTransformMatrixOperation::~ApplyTransformMatrixOperation() {}
  vtkSmartPointer<vtkMatrix4x4> ApplyTransformMatrixOperation::GetMatrix() { return m_vtkMatrix; }
  Point3D ApplyTransformMatrixOperation::GetReferencePoint() { return m_referencePoint; }
} // namespace mitk
