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

#include "mitkUSDiPhASDopplerControls.h"
#include "mitkUSDiPhASDevice.h"
#include <mitkException.h>

mitk::USDiPhASDopplerControls::USDiPhASDopplerControls(itk::SmartPointer<USDiPhASDevice> device)
  : mitk::USControlInterfaceDoppler(device.GetPointer())
{
}

mitk::USDiPhASDopplerControls::~USDiPhASDopplerControls()
{
}

void mitk::USDiPhASDopplerControls::SetIsActive(bool isActive)
{
  m_IsActive = isActive;
}

bool mitk::USDiPhASDopplerControls::GetIsActive()
{
  return m_IsActive;
}