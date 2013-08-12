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
#include <usModuleResource.h>

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


  protected:

   /**
   \brief Returns the CameraIntrinsics for the cameras created by this factory.
   *
   * This Method calls the virtual method GetIntrinsicsResource() to retrieve the necessary data.
   * Override getIntrinsicsResource in your subclasses, also see the documentation of GetIntrinsicsResource
   */
   CameraIntrinsics::Pointer GetCameraIntrinsics();

   /**
   \brief Returns the ModuleResource that contains a xml definition of the CameraIntrinsics.
   *
   * The default implementation returns a default calibration.
   * In subclasses, you can override this method to return a different xml resource.
   * See this implementation for an example.
   */
   virtual us::ModuleResource GetIntrinsicsResource();


   std::vector<ToFCameraDevice::Pointer> m_Devices;
   std::map<ToFCameraDevice*,us::ServiceRegistration<ToFCameraDevice> > m_DeviceRegistrations;
};
}
#endif
