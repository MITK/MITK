/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkAbstractToFDeviceFactory_h
#define mitkAbstractToFDeviceFactory_h

#include <MitkToFHardwareExports.h>
#include "mitkIToFDeviceFactory.h"

// Microservices
#include <usServiceRegistration.h>
#include <usModuleResource.h>

namespace mitk
{
  /**
  * @brief Virtual interface and base class for all Time-of-Flight device factories.
  * The basic interface is in the base class: IToFDeviceFactory. This
  * AbstractToFDeviceFactory implements some generic function which are useful
  * for every device and not specific.
  *
  * @ingroup ToFHardware
  */
struct MITKTOFHARDWARE_EXPORT AbstractToFDeviceFactory : public IToFDeviceFactory {
   public:

  /**
    * @brief ConnectToFDevice Use this method to connect a device.
    * @return A ToFCameraDevice.
    */
   ToFCameraDevice::Pointer ConnectToFDevice();

   /**
    * @brief DisconnectToFDevice Use this method to disconnect a device.
    * @param device The device you want to disconnect.
    */
   void DisconnectToFDevice(const ToFCameraDevice::Pointer& device);

  /**
    * @brief GetNumberOfDevices Get the number of devices produced by this factory.
    * This function will return the number of registered devices to account for
    * created and disconnected devices.
    *
    * @return Number of registered devices.
    */
  size_t GetNumberOfDevices();

  std::string GetCurrentDeviceName() override
  {
    std::stringstream name;
    if(this->GetNumberOfDevices()>1)
    {
      name << this->GetDeviceNamePrefix()<< " "<< this->GetNumberOfDevices();
    }
    else
    {
      name << this->GetDeviceNamePrefix();
    }
    return name.str();
  }

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

   /**
    * @brief m_Devices A list (vector) containing all connected devices of
    * the respective factory.
    */
   std::vector<ToFCameraDevice::Pointer> m_Devices;
   /**
    * @brief m_DeviceRegistrations A map containing all the pairs of
    * device registration numbers and devices.
    */
   std::map<ToFCameraDevice*,us::ServiceRegistration<ToFCameraDevice> > m_DeviceRegistrations;
};
}
#endif
