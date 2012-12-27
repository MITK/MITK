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


#include "mitkModeOperation.h"
#include <mitkOperation.h>

mitk::ModeOperation::ModeOperation(mitk::OperationType operationType, mitk::ModeOperation::ModeType mode)
: Operation(operationType), m_Mode(mode)
{
}

mitk::ModeOperation::~ModeOperation()
{
}

mitk::ModeOperation::ModeType mitk::ModeOperation::GetMode()
{
  return m_Mode;
}
