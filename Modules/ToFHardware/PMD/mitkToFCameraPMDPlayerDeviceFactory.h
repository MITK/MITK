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

#include "mitkPMDModuleExports.h"
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
class MITK_PMDMODULE_EXPORT ToFCameraPMDPlayerDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
  /**
   * @brief ToFCameraPMDPlayerDeviceFactory Default contructor.
   * This factory internally counts all PMD Player devices starting at 1.
   */
  ToFCameraPMDPlayerDeviceFactory()
  {
    this->m_DeviceNumber=1;
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
  std::string GetCurrentDeviceName()
  {
    std::stringstream name;
    if(m_DeviceNumber>1)
    {
      name << "PMD Player "<< m_DeviceNumber;
    }
    else
    {
      name << "PMD Player";
    }
    m_DeviceNumber++;
    return name.str();
  }

private:
  /*!
    \brief Create an instance of a ToFCameraPMDPlayerDevice.
    Sets default properties for a PMD Player.
    */
  ToFCameraDevice::Pointer CreateToFCameraDevice()
  {
    ToFCameraPMDPlayerDevice::Pointer device = ToFCameraPMDPlayerDevice::New();

    device->SetBoolProperty("HasRGBImage", false);
    device->SetBoolProperty("HasAmplitudeImage", true);
    device->SetBoolProperty("HasIntensityImage", true);

    return device.GetPointer();
  }

  /**
   * @brief m_DeviceNumber Member for counting of devices.
   */
  int m_DeviceNumber;
};
}
#endif
