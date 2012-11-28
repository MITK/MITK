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

namespace mitk
{
  /**
  * @brief Virtual interface and base class for all Time-of-Flight device factories
  *
  * @ingroup ToFHardware
  */

struct MITK_TOFHARDWARE_EXPORT AbstractToFDeviceFactory : public IToFDeviceFactory {
   public:

   ToFCameraDevice::Pointer ConnectToFDevice();

   void DisconnectToFDevice(const ToFCameraDevice::Pointer& device);

private:

   std::vector<ToFCameraDevice::Pointer> m_Devices;
   std::map<ToFCameraDevice*,ServiceRegistration> m_DeviceRegistrations;
};
}
#endif
