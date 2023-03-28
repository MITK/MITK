/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFCameraPMDO3DeviceFactory_h
#define mitkToFCameraPMDO3DeviceFactory_h

#include <MitkPMDExports.h>
#include "mitkToFCameraPMDO3Device.h"
#include "mitkAbstractToFDeviceFactory.h"
#include <mitkCameraIntrinsics.h>
#include <mitkCameraIntrinsicsProperty.h>
#include <mitkToFConfig.h>


namespace mitk
{
  /**
  * \brief ToFCameraPMDO3DeviceFactory is an implementation of the factory pattern to generate Do3 Devices.
  * ToFPMDCamCubeDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Cam Cube Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITKPMD_EXPORT ToFCameraPMDO3DeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
    ToFCameraPMDO3DeviceFactory()
    {
    }

   /*!
   \brief Defining the Factories Name, here for the ToFPMDO3Device
   */
   std::string GetFactoryName()
   {
       return std::string("PMD O3D Factory");
   }

   /**
    * @brief GetFactoryName Main part of the device name.
    */
   std::string GetFactoryName()
   {
       return std::string("PMD O3");
   }

private:
     /*!
   \brief Create an instance of a ToFPMDO3DeviceFactory.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraPMDO3Device::Pointer device = ToFCameraPMDO3Device::New();

     device->SetBoolProperty("HasRGBImage", false);
     device->SetBoolProperty("HasAmplitudeImage", true);
     device->SetBoolProperty("HasIntensityImage", true);

     return device.GetPointer();
   }
};
}
#endif
