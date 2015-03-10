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
#ifndef __mitkToFCameraPMDPlayerDeviceFactory_h
#define __mitkToFCameraPMDPlayerDeviceFactory_h

#include <MitkPMDExports.h>
#include "mitkToFCameraPMDPlayerDevice.h"
#include "mitkAbstractToFDeviceFactory.h"
#include <mitkCameraIntrinsics.h>
#include <mitkCameraIntrinsicsProperty.h>
#include <mitkToFConfig.h>

namespace mitk
{
  /**
  * \brief ToFPMDPlayerDeviceFactory is an implementation of the factory pattern to generate PMD Player Devices.
  * ToFPMDPlayerDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new PMD Player Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */
class MITKPMD_EXPORT ToFCameraPMDPlayerDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
  /**
   * @brief ToFCameraPMDPlayerDeviceFactory Default contructor.
   * This factory internally counts all PMD Player devices starting at 1.
   */
  ToFCameraPMDPlayerDeviceFactory()
  {
  }
  /*!
   \brief Get the name of the factory, here for the ToFPMDPlayer.
   */
   std::string GetFactoryName()
   {
     return std::string("PMD Player Factory");
   }

  /**
   * @brief GetCurrentDeviceName Get the name of the current Cam Cube.
   * First device is named "PMD Player", second "PMD Player 2" and so on.
   * @return Human readable name as string.
   */
   std::string GetDeviceNamePrefix()
   {
     return std::string("PMD Player");
   }

private:
     /*!
   \brief Create an instance of a ToFPMDPlayerDevice.
   */
   ToFCameraDevice::Pointer CreateToFCameraDevice()
   {
     ToFCameraPMDPlayerDevice::Pointer device = ToFCameraPMDPlayerDevice::New();

      device->SetBoolProperty("HasRGBImage", false);
      device->SetBoolProperty("HasAmplitudeImage", true);
      device->SetBoolProperty("HasIntensityImage", true);

     return device.GetPointer();
   }
};
}
#endif
