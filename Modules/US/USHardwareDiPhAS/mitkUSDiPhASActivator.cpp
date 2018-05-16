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