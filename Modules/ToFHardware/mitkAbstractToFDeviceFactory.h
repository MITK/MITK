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
#ifndef __mitkAbstractToFDeviceFactory_h
#define __mitkAbstractToFDeviceFactory_h

#include "mitkToFHardwareExports.h"
#include "mitkIToFDeviceFactory.h"

// Microservices
#include <usServiceRegistration.h>
#include <usGetModuleContext.h>
#include "mitkModuleContext.h"

namespace mitk
{
  /**
  * @brief Virtual interface and base class for all Time-of-Flight device factories
  *
  * @ingroup ToFHardware
  */

struct MITK_TOFHARDWARE_EXPORT AbstractToFDeviceFactory : public IToFDeviceFactory {
   public:

   ToFCameraDevice::Pointer ConnectToFDevice()
   {
      ToFCameraDevice::Pointer device = createToFCameraDevice();
      m_Devices.push_back(device);

     ModuleContext* context = GetModuleContext();
     ServiceProperties deviceProps;
//-------------Take a look at this part to change the name given to a device
     deviceProps["ToFDeviceName"] = GetFactoryName() + " Device";
     m_DeviceRegistrations.insert(std::make_pair(device.GetPointer(), context->RegisterService<ToFCameraDevice>(device.GetPointer(),deviceProps)));

     return device;
   }

   void DisconnectToFDevice(const ToFCameraDevice::Pointer& device)
   {
      std::map<ToFCameraDevice*,ServiceRegistration>::iterator i = m_DeviceRegistrations.find(device.GetPointer());
      if (i == m_DeviceRegistrations.end()) return;

      i->second.Unregister();
      m_DeviceRegistrations.erase(i);

      m_Devices.erase(std::remove(m_Devices.begin(), m_Devices.end(), device), m_Devices.end());
   }

private:

   std::vector<ToFCameraDevice::Pointer> m_Devices;
   std::map<ToFCameraDevice*,ServiceRegistration> m_DeviceRegistrations;
};
}
#endif
