/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkIToFDeviceFactory_h
#define mitkIToFDeviceFactory_h

#include "mitkCameraIntrinsics.h"
#include "mitkToFCameraDevice.h"
#include <MitkToFHardwareExports.h>
//for microservices
#include <mitkServiceInterface.h>

namespace mitk
{
  /**
  \brief  This is the base of class for all ToFDeviceFactories.

  It is an interface of the factory pattern. You can create a
  specialized ToF device factory in the inherited class.
  */
  struct MITKTOFHARDWARE_EXPORT IToFDeviceFactory {
    virtual ~IToFDeviceFactory();

    /**
     * @brief GetFactoryName Get the human readable name of the factory.
     * @return Human readable name as string.
     */
    virtual std::string GetFactoryName() = 0;

    /**
     * @brief GetCurrentDeviceName Get the human readable name of the current
     * device. A factory can produce many device and give ordered names for each
     * device (e.g. number them like Device_1, Device_2, etc.).
     * @return The human readable name of the current device as string.
     */
    virtual std::string GetCurrentDeviceName() = 0;

    virtual std::string GetDeviceNamePrefix() = 0;

    /**
     * @brief CreateToFCameraDevice Create a new device of the respective
     * factory. E.g. a "KinectFactory" creates a KinectDevice.
     * @return A ToFCameraDevice.
     */
    virtual ToFCameraDevice::Pointer CreateToFCameraDevice() = 0;

  };
}

/**
IToFDeviceFactory is declared a MicroService interface. See
MicroService documenation for more details.
*/
MITK_DECLARE_SERVICE_INTERFACE(mitk::IToFDeviceFactory, "org.mitk.services.IToFDeviceFactory")
#endif
