/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSControlInterfaceDoppler.h"
#include "mitkUSDevice.h"

mitk::USControlInterfaceDoppler::USControlInterfaceDoppler(itk::SmartPointer<USDevice> device)
  : mitk::USAbstractControlInterface(device)
{
}

mitk::USControlInterfaceDoppler::~USControlInterfaceDoppler()
{
}
