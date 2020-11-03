/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSDiPhASActivator.h"

mitk::USDiPhASActivator::USDiPhASActivator()
{
  MITK_INFO << "USDiPhASActivator created";
}

mitk::USDiPhASActivator::~USDiPhASActivator()
{
}

void mitk::USDiPhASActivator::Load(us::ModuleContext* context)
{
  // create a new device
  m_Device = mitk::USDiPhASDevice::New("DiPhAS", "Ultrasound System");
  m_Device->Initialize();
}

void mitk::USDiPhASActivator::Unload(us::ModuleContext* context)
{
  // set smart pointer to null (device will be unregistered from
  // micro service in it's destrcutor)
  m_Device = 0;
}
