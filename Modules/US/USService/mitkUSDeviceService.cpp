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

#include "mitkUSDeviceService.h"
#include <string>


mitk::USDeviceService::USDeviceService() : itk::Object()
{

}

mitk::USDeviceService::~USDeviceService()
{

}

void mitk::USDeviceService::ActivateDevice (mitk::USDevice::Pointer device){
  // Check if device is already active
  for(std::vector<mitk::USDevice::Pointer>::iterator it = m_ActiveDevices.begin(); it != m_ActiveDevices.end(); ++it) {
    if (it->GetPointer() == device.GetPointer()) return;
  }

  // add device
  m_ActiveDevices.push_back(device);
}


void mitk::USDeviceService::DeactivateDevice (int index){
  // m_ActiveDevices.erase(index);
  // Not yet supported
}

std::vector<mitk::USDevice::Pointer> mitk::USDeviceService::GetActiveDevices(){
  return m_ActiveDevices;
}