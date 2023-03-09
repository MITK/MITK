/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFCameraMESASR4000DeviceFactory_h
#define mitkToFCameraMESASR4000DeviceFactory_h

#include <MitkMESASR4000ModuleExports.h>
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
  * This offers users the oppertunity to generate new MESA SR4000 Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */
  class MITKMESASR4000_EXPORT ToFCameraMESASR4000DeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

  public:
    /**
     * @brief ToFCameraMESASR4000DeviceFactory Default contructor.
     * This factory internally counts all MESA SR4000 devices starting at 1.
     */
    ToFCameraMESASR4000DeviceFactory()
    {
    }
    /*!
     \brief Get the name of the factory, here for the ToFPMDCamCube.
     */
    std::string GetFactoryName()
    {
      return std::string("MESA SR4000 Factory");
    }

    /**
    * @brief GetCurrentDeviceName Get the name of the current MESA SR4000.
    * First device is named "MESA SR4000", second "MESA SR4000 2" and so on.
    * @return Human readable name as string.
    */
    std::string GetCurrentDeviceName()
    {
      return std::string("MESA SR4000");
    }

  private:
    /*!
     \brief Create an instance of a ToFCameraMESASR4000Device.
     Sets default properties for a MESA SR4000.
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
  };
}
#endif
