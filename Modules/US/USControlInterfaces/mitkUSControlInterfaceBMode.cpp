/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSControlInterfaceBMode.h"
#include "mitkUSDevice.h"

mitk::USControlInterfaceBMode::USControlInterfaceBMode( itk::SmartPointer<USDevice> device )
  : mitk::USAbstractControlInterface(device)
{
  if (device.IsNull())
  {
    MITK_ERROR << "USDevice must not be null for USControlInterfaceBMode.";
    mitkThrow() << "USDevice must not be null for USControlInterfaceBMode.";
  }
}

mitk::USControlInterfaceBMode::~USControlInterfaceBMode()
{
}

void mitk::USControlInterfaceBMode::Initialize()
{
  m_Device->UpdateServiceProperty(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_FREQUENCY,
    this->GetScanningFrequency());

  m_Device->UpdateServiceProperty(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_POWER,
    this->GetScanningPower());

  m_Device->UpdateServiceProperty(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH,
    this->GetScanningDepth());

  m_Device->UpdateServiceProperty(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_GAIN,
    this->GetScanningGain());

  m_Device->UpdateServiceProperty(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_REJECTION,
    this->GetScanningRejection());

  m_Device->UpdateServiceProperty(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DYNAMIC_RANGE,
    this->GetScanningDynamicRange());
}

void mitk::USControlInterfaceBMode::SetScanningFrequency( double frequency )
{
  this->OnSetScanningFrequency(frequency);
  m_Device->UpdateServiceProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_FREQUENCY, frequency);
}

void mitk::USControlInterfaceBMode::SetScanningPower( double power )
{
  this->OnSetScanningPower(power);
  m_Device->UpdateServiceProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_POWER, power);
}

void mitk::USControlInterfaceBMode::SetScanningDepth( double depth )
{
  this->OnSetScanningDepth(depth);
  m_Device->UpdateServiceProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DEPTH, depth);
}

void mitk::USControlInterfaceBMode::SetScanningGain( double gain )
{
  this->OnSetScanningGain(gain);
  m_Device->UpdateServiceProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_GAIN, gain);
}

void mitk::USControlInterfaceBMode::SetScanningRejection( double rejection )
{
  this->OnSetScanningRejection(rejection);
  m_Device->UpdateServiceProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_REJECTION, rejection);
}

void mitk::USControlInterfaceBMode::SetScanningDynamicRange( double dynamicRange )
{
  this->OnSetScanningDynamicRange(dynamicRange);
  m_Device->UpdateServiceProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DYNAMIC_RANGE, dynamicRange);
}
