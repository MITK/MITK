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

#include "mitkMESASR4000ModuleExports.h"
#include "mitkToFCameraMESASR4000Device.h"
#include "mitkAbstractToFDeviceFactory.h"
#include <mitkCameraIntrinsics.h>
#include <mitkCameraIntrinsicsProperty.h>
#include <mitkToFConfig.h>

namespace mitk
{
  /**
  * \brief ToFCameraMESASR4000DeviceFactory is an implementation of the factory pattern to generate MESASR4000Devices.
  * ToFCameraMESASR4000DeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Raw Player Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

  class MITK_MESASR4000MODULE_EXPORT ToFCameraMESASR4000DeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

  public:
    ToFCameraMESASR4000DeviceFactory()
    {
      this->m_DeviceNumber = 1;
    }
    /*!
    \brief Defining the Factorie�s Name, here for the MESASR4000DeviceFactory
    */
    std::string GetFactoryName()
    {
      return std::string("MESA SR4000 Factory");
    }
    std::string GetCurrentDeviceName()
    {
      std::stringstream name;
      if(m_DeviceNumber>1)
      {
        name << "MESA SR4000 "<< m_DeviceNumber;
      }
      else
      {
        name << "MESA SR4000";
      }
      m_DeviceNumber++;
      return name.str();
    }

  private:
    /*!
    \brief Create an instance of a ToFCameraMESASR4000Device.
    */
    ToFCameraDevice::Pointer CreateToFCameraDevice()
    {
      ToFCameraMESASR4000Device::Pointer device = ToFCameraMESASR4000Device::New();

      //Set default camera intrinsics for the Mesa-SR4000-camera.
      mitk::CameraIntrinsics::Pointer cameraIntrinsics = mitk::CameraIntrinsics::New();
      std::string pathToDefaulCalibrationFile(MITK_TOF_DATA_DIR);

      pathToDefaulCalibrationFile.append("/CalibrationFiles/Mesa-SR4000_Camera.xml");
      cameraIntrinsics->FromXMLFile(pathToDefaulCalibrationFile);
      device->SetProperty("CameraIntrinsics", mitk::CameraIntrinsicsProperty::New(cameraIntrinsics));

      device->SetBoolProperty("HasRGBImage", false);
      device->SetBoolProperty("HasAmplitudeImage", true);
      device->SetBoolProperty("HasIntensityImage", true);

      return device.GetPointer();
    }
    int m_DeviceNumber;
  };
}
#endif
