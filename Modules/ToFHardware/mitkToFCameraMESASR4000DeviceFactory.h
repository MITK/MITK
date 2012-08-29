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
#ifndef __mitkToFCameraMESASR4000DeviceFactory_h
#define __mitkToFCameraMESASR4000DeviceFactory_h

#include "mitkToFHardwareExports.h"
#include "mitkToFCameraMESASR4000Device.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief ToFPMDRawPlayerDeviceFactory is an implementation of the factory pattern to generate MESASR4000Devices.
  * ToFCameraMESASR4000DeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Raw Player Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_TOFHARDWARE_EXPORT ToFCameraMESASR4000DeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
   /*!
   \brief Defining the Factorie´s Name, here for the MESASR4000DeviceFactory
   */
   std::string GetFactoryName()
   {
       return std::string("MESA SR4000 Device Factory");
   }

private:
     /*!
   \brief Create an instance of a ToFPMDRawDataDevice.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraMESASR4000Device::Pointer device = ToFCameraMESASR4000Device::New();

     return device.GetPointer();
   }
};
}
#endif
