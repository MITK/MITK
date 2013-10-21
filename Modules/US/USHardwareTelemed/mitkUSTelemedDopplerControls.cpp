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

#include "mitkUSTelemedDopplerControls.h"
#include "mitkUSTelemedDevice.h"
#include <mitkException.h>

mitk::USTelemedDopplerControls::USTelemedDopplerControls(itk::SmartPointer<USTelemedDevice> device)
  : mitk::USControlInterfaceDoppler(device.GetPointer())
{
}

mitk::USTelemedDopplerControls::~USTelemedDopplerControls()
{
}

void mitk::USTelemedDopplerControls::SetIsActive(bool isActive)
{
  m_IsActive = isActive;
}

bool mitk::USTelemedDopplerControls::GetIsActive()
{
  return m_IsActive;
}