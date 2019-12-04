/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOperation.h"

mitk::Operation::Operation(mitk::OperationType operationType) : m_OperationType(operationType)
{
}

mitk::Operation::~Operation()
{
}

mitk::OperationType mitk::Operation::GetOperationType()
{
  return m_OperationType;
}
