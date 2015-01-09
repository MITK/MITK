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


#ifndef mitkApplyTransformMatrixOperation_h_Included
#define mitkApplyTransformMatrixOperation_h_Included

#include "mitkCommon.h"
#include "mitkPointOperation.h"

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>



namespace mitk {

class MITKCORE_EXPORT ApplyTransformMatrixOperation : public Operation
{
  public:
  //##Documentation
  //##@brief Operation that applies a new vtk transform matrix.
  //##
  //## @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
  //## @param matrix is the vtk 4x4 vtk matrix of the transformation
  //## @param refPoint is the reference point for realigning the plane stack

    ApplyTransformMatrixOperation(OperationType operationType, vtkSmartPointer<vtkMatrix4x4> matrix, mitk::Point3D refPoint);

    virtual ~ApplyTransformMatrixOperation();

    vtkSmartPointer<vtkMatrix4x4> GetMatrix();

    mitk::Point3D GetReferencePoint();

  private:

    vtkSmartPointer<vtkMatrix4x4> m_vtkMatrix;
    mitk::Point3D m_referencePoint;

};
}//namespace mitk
#endif
