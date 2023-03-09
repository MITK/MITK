/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFCameraPMDCamCubeDeviceFactory_h
#define mitkToFCameraPMDCamCubeDeviceFactory_h

#include <MitkPMDExports.h>
#include "mitkToFCameraPMDCamCubeDevice.h"
#include "mitkAbstractToFDeviceFactory.h"
#include <mitkCameraIntrinsics.h>
#include <mitkCameraIntrinsicsProperty.h>
#include <mitkToFConfig.h>

// Microservices
#include <usServiceRegistration.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

namespace mitk
{
  /**
  * \brief ToFPMDCamBoardDeviceFactory is an implementation of the factory pattern to generate Cam Cube Devices.
  * ToFPMDCamCubeDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Cam Cube Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITKPMD_EXPORT ToFCameraPMDCamCubeDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
  /**
   * @brief ToFCameraPMDCamCubeDeviceFactory Default contructor.
   * This factory internally counts all Cam Cube devices starting at 1.
   */
  ToFCameraPMDCamCubeDeviceFactory()
  {
  }
  /**
  * @brief GetCurrentDeviceName Get the name of the current Cam Cube.
  * First device is named "PMD CamCube 2.0/3.0", second "PMD CamCube 2.0/3.0 2" and so on.
  * @return Human readable name as string.
  */
   std::string GetFactoryName()
   {
     return std::string("PMD Camcube 2.0/3.0 Factory ");
   }

   /**
    * @brief GetDeviceNamePrefix Main part of the device name.
    */
   std::string GetDeviceNamePrefix()
   {
     return std::string("PMD CamCube 2.0/3.0");
   }

private:
  /*!
   \brief Create an instance of a ToFPMDCamCubeDevice.
   */
   ToFCameraDevice::Pointer CreateToFCameraDevice()
   {
     ToFCameraPMDCamCubeDevice::Pointer device = ToFCameraPMDCamCubeDevice::New();

      device->SetBoolProperty("HasRGBImage", false);
      device->SetBoolProperty("HasAmplitudeImage", true);
      device->SetBoolProperty("HasIntensityImage", true);

     return device.GetPointer();
   }

  /**
    * @brief GetIntrinsicsResource Get the resource path to a default
    * camera intrinsics .xml file.
    * @return Path to the intrinsics .xml file.
    */
  us::ModuleResource GetIntrinsicsResource()
  {
    us::Module* module = us::GetModuleContext()->GetModule();
    return module->GetResource("CalibrationFiles/PMDCamCube3_camera.xml");
  }

  /**
  * @brief m_DeviceNumber Member for counting of devices.
  */
  int m_DeviceNumber;
};
}
#endif
