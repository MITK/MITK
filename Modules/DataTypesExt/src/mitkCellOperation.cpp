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
