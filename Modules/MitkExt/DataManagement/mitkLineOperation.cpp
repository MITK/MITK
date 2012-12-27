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


#include "mitkLineOperation.h"

mitk::LineOperation::LineOperation(OperationType operationType, int cellId, Vector3D vector, int pIdA, int pIdB, int id)
: mitk::CellOperation(operationType, cellId, vector), m_PIdA(pIdA), m_PIdB(pIdB), m_Id(id)
{}
mitk::LineOperation::LineOperation(OperationType operationType, int cellId, int pIdA, int pIdB, int id)
: mitk::CellOperation(operationType, cellId), m_PIdA(pIdA), m_PIdB(pIdB), m_Id(id)
{}

int mitk::LineOperation::GetPIdA()
{
  return m_PIdA;
}

int mitk::LineOperation::GetPIdB()
{
  return m_PIdB;
}

int mitk::LineOperation::GetId()
{
  return m_Id;
}


