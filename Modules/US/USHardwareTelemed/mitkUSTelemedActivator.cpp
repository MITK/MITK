/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSTelemedActivator.h"

mitk::USTelemedActivator::USTelemedActivator()
{
  MITK_INFO << "USTelemedActivator created";
}

mitk::USTelemedActivator::~USTelemedActivator()
{
}

void mitk::USTelemedActivator::Load(us::ModuleContext* context)
{
  // create a new device
  m_Device = mitk::USTelemedDevice::New("Telemed", "Ultrasound System");
  m_Device->Initialize();
}

void mitk::USTelemedActivator::Unload(us::ModuleContext* context)
{
  // set smart pointer to null (device will be unregistered from
  // micro service in it's destrcutor)
  m_Device = 0;
}
