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
	: mitk::USDevice(manufacturer, model), m_ControlsProbes(mitk::USFraunhoferProbesControls::New(this)),
	m_ControlsBMode(mitk::USFraunhoferBModeControls::New(this)),
	m_ControlsDoppler(mitk::USFraunhoferDopplerControls::New(this)),
	m_ImageSource(mitk::USFraunhoferImageSource::New())
{
  SetNumberOfOutputs(1);
  SetNthOutput(0, this->MakeOutput(0));
}

mitk::USFraunhoferDevice::~USFraunhoferDevice()
{
}

std::string mitk::USFraunhoferDevice::GetDeviceClass()
{
  return "org.mitk.modules.us.USFraunhoferDevice";
}

mitk::USControlInterfaceBMode::Pointer mitk::USFraunhoferDevice::GetControlInterfaceBMode()
{
  return m_ControlsBMode.GetPointer();
}

mitk::USControlInterfaceProbes::Pointer mitk::USFraunhoferDevice::GetControlInterfaceProbes()
{
  return m_ControlsProbes.GetPointer();
};

mitk::USControlInterfaceDoppler::Pointer mitk::USFraunhoferDevice::GetControlInterfaceDoppler()
{
  return m_ControlsDoppler.GetPointer();
};

bool mitk::USFraunhoferDevice::OnInitialization()
{
  return true;
}

bool mitk::USFraunhoferDevice::OnConnection()
{
  // Setup the scanmode
  return true;
}

bool mitk::USFraunhoferDevice::OnDisconnection()
{
  // control objects cannot be active anymore


  return true;
}

bool mitk::USFraunhoferDevice::OnActivation()
{
  // probe controls are available now

  return true;
}

bool mitk::USFraunhoferDevice::OnDeactivation()
{
  return true;
}

void mitk::USFraunhoferDevice::OnFreeze(bool freeze)
{

}

mitk::USImageSource::Pointer mitk::USFraunhoferDevice::GetUSImageSource()
{
  return m_ImageSource;
}

ScanModeNative& mitk::USFraunhoferDevice::GetScanMode()
{
  return m_ScanMode;
}