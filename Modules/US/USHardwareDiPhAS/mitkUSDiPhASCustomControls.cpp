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

#include "mitkUSDiPhASCustomControls.h"

mitk::USDiPhASCustomControls::USDiPhASCustomControls(USDiPhASDevice* device)
  : mitk::USDiPhASDeviceCustomControls(device), m_IsActive(false), m_device(device)
{
}

mitk::USDiPhASCustomControls::~USDiPhASCustomControls()
{
}

void mitk::USDiPhASCustomControls::SetIsActive(bool isActive)
{
  m_IsActive = isActive;
}

bool mitk::USDiPhASCustomControls::GetIsActive()
{
  return m_IsActive;
}

void mitk::USDiPhASCustomControls::OnSetSoundOfSpeed(int speed)
{
  m_device->GetScanMode().averageSpeedOfSound = speed;
  m_device->UpdateScanmode();
}