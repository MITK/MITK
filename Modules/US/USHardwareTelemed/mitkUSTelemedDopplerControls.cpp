/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
