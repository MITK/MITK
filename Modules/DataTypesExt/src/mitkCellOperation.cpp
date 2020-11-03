/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCellOperation.h"

mitk::CellOperation::CellOperation(OperationType operationType, int cellId, Vector3D vector)
  : mitk::Operation(operationType), m_CellId(cellId), m_Vector(vector)
{
}

mitk::CellOperation::CellOperation(OperationType operationType, int cellId)
  : mitk::Operation(operationType), m_CellId(cellId)
{
  m_Vector.Fill(0);
}
