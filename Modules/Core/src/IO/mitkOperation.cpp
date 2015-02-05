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


#include "mitkOperation.h"

mitk::Operation::Operation(mitk::OperationType operationType)
: m_OperationType(operationType)
{
}

mitk::Operation::~Operation()
{
}

mitk::OperationType mitk::Operation::GetOperationType()
{
  return m_OperationType;
}
