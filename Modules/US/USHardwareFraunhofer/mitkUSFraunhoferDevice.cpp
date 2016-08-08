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

#include "mitkUSFraunhoferDevice.h"

//#include "mitkUSFraunhoferSDKHeader.h"

mitk::USFraunhoferDevice::USFraunhoferDevice(std::string manufacturer, std::string model)
: mitk::USDevice(manufacturer, model){}

mitk::USFraunhoferDevice::~USFraunhoferDevice()
{
}

std::string mitk::USFraunhoferDevice::GetDeviceClass()
{
  return "org.mitk.modules.us.USFraunhoferDevice";
}

mitk::USControlInterfaceBMode::Pointer mitk::USFraunhoferDevice::GetControlInterfaceBMode()
{
  return nullptr;
}

mitk::USControlInterfaceProbes::Pointer mitk::USFraunhoferDevice::GetControlInterfaceProbes()
{
  return nullptr;
};

mitk::USControlInterfaceDoppler::Pointer mitk::USFraunhoferDevice::GetControlInterfaceDoppler()
{
  return nullptr;
};

bool mitk::USFraunhoferDevice::OnInitialization()
{

  return true;
}

bool mitk::USFraunhoferDevice::OnConnection()
{
  // create main Fraunhofer API COM library object

  return true;
}

bool mitk::USFraunhoferDevice::OnDisconnection()
{
  // control objects cannot be active anymore

  ReleaseUsgControls();

  return true;
}

bool mitk::USFraunhoferDevice::OnActivation()
{
  // probe controls are available now

  return true;
}

bool mitk::USFraunhoferDevice::OnDeactivation()
{
  this->StopScanning();
  return true;
}

void mitk::USFraunhoferDevice::OnFreeze(bool freeze)
{

}

mitk::USImageSource::Pointer mitk::USFraunhoferDevice::GetUSImageSource()
{
  return nullptr;
}

void mitk::USFraunhoferDevice::ReleaseUsgControls()
{

}

void mitk::USFraunhoferDevice::StopScanning()
{

}