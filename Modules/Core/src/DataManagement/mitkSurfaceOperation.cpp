/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceOperation.h"

mitk::SurfaceOperation::SurfaceOperation(mitk::OperationType operationType, vtkPolyData *polyData, unsigned int t)
  : mitk::Operation(operationType), m_polyData(polyData), m_timeStep(t)
{
}

mitk::SurfaceOperation::~SurfaceOperation()
{
}

vtkPolyData *mitk::SurfaceOperation::GetVtkPolyData()
{
  return m_polyData;
}

unsigned int mitk::SurfaceOperation::GetTimeStep()
{
  return m_timeStep;
}
