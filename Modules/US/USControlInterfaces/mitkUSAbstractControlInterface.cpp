/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSAbstractControlInterface.h"
#include "mitkUSDevice.h"

mitk::USAbstractControlInterface::USAbstractControlInterface(itk::SmartPointer<USDevice> device)
  : m_Device(device)
{
}

mitk::USAbstractControlInterface::~USAbstractControlInterface()
{
}
