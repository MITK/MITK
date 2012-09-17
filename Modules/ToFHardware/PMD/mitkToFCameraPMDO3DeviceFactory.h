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
#ifndef __mitkToFCameraPMDO3DeviceFactory_h
#define __mitkToFCameraPMDO3DeviceFactory_h

#include "mitkPMDCamCubeModuleExports.h"
#include "mitkToFCameraPMDO3Device.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief ToFCameraPMDO3DeviceFactory is an implementation of the factory pattern to generate Do3 Devices.
  * ToFPMDCamCubeDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Cam Cube Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_PMDCAMCUBEMODULE_EXPORT ToFCameraPMDO3DeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
   /*!
   \brief Defining the Factorie´s Name, here for the ToFPMDO3Device
   */
   std::string GetFactoryName()
   {
       return std::string("PMD O3D Factory");
   }

private:
     /*!
   \brief Create an instance of a ToFPMDO3DeviceFactory.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraPMDO3Device::Pointer device = ToFCameraPMDO3Device::New();

     return device.GetPointer();
   }

};
}
#endif
