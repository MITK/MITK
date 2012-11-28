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
#ifndef __mitkIToFDeviceFactory_h
#define __mitkIToFDeviceFactory_h

#include "mitkToFCameraDevice.h"
#include "mitkToFHardwareExports.h"
//for microservices
#include <usServiceInterface.h>

namespace mitk
{
  struct MITK_TOFHARDWARE_EXPORT IToFDeviceFactory {

    virtual ~IToFDeviceFactory(); // leer in mitkIToFDeviceFactory.cpp implementieren

    //create a specialized device factory in the inherited class

    virtual std::string GetFactoryName() = 0;

    virtual std::string GetCurrentDeviceName() = 0;

    virtual ToFCameraDevice::Pointer createToFCameraDevice() = 0;
    //create a specialized device factory in the inherited class


  };
}
US_DECLARE_SERVICE_INTERFACE(mitk::IToFDeviceFactory, "org.mitk.services.IToFDeviceFactory")
#endif
