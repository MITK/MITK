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

#include "mitkUSFraunhoferDopplerControls.h"
#include "mitkUSFraunhoferDevice.h"
#include <mitkException.h>

mitk::USFraunhoferDopplerControls::USFraunhoferDopplerControls(itk::SmartPointer<USFraunhoferDevice> device)
  : mitk::USControlInterfaceDoppler(device.GetPointer())
{
}

mitk::USFraunhoferDopplerControls::~USFraunhoferDopplerControls()
{
}

void mitk::USFraunhoferDopplerControls::SetIsActive(bool isActive)
{
  m_IsActive = isActive;
}

bool mitk::USFraunhoferDopplerControls::GetIsActive()
{
  return m_IsActive;
}