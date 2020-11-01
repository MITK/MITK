/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkApplyTransformMatrixOperation_h_Included
#define mitkApplyTransformMatrixOperation_h_Included

#include "mitkCommon.h"
#include "mitkPointOperation.h"

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  class MITKCORE_EXPORT ApplyTransformMatrixOperation : public Operation
  {
  public:
    /** @brief Operation that applies a new vtk transform matrix.
      *
      * @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
      * @param matrix is the vtk 4x4 vtk matrix of the transformation
      * @param refPoint is the reference point for realigning the plane stack
      */
    ApplyTransformMatrixOperation(OperationType operationType,
                                  vtkSmartPointer<vtkMatrix4x4> matrix,
                                  mitk::Point3D refPoint);

    ~ApplyTransformMatrixOperation() override;

    vtkSmartPointer<vtkMatrix4x4> GetMatrix();

    mitk::Point3D GetReferencePoint();

  private:
    vtkSmartPointer<vtkMatrix4x4> m_vtkMatrix;
    mitk::Point3D m_referencePoint;
  };
} // namespace mitk
#endif
