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
#ifndef __mitkToFCameraMESADeviceFactory_h
#define __mitkToFCameraMESADeviceFactory_h

#include "mitkToFHardwareExports.h"
#include "mitkToFCameraMESADevice.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief ToFCameraMESASR4000DeviceFactory is an implementation of the factory pattern to generate MESASR4000 Devices.
  * ToFCameraMESASR4000DeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new MESASR4000 Device via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_TOFHARDWARE_EXPORT ToFCameraMESADeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
   /*!
   \brief Defining the Factorie´s Name, here for the ToFCameraMESADeviceFactory.
   */
   std::string GetFactoryName()
   {
       return std::string("MESA Device Factory");
   }

private:
     /*!
   \brief Create an instance of a ToFCameraMESADeviceFactory.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraMESADevice::Pointer device = ToFCameraMESADevice::New();

     return device.GetPointer();
   }
};
}
#endif
