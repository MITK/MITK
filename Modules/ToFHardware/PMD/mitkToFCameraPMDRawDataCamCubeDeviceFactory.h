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
#ifndef __mitkToFCameraPMDRawDataCamCubeDeviceFactory_h
#define __mitkToFCameraPMDRawDataCamCubeDeviceFactory_h

#include <MitkPMDExports.h>
#include "mitkToFCameraPMDRawDataCamCubeDevice.h"
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
  * \brief ToFPMDRawPlayerDeviceFactory is an implementation of the factory pattern to generate Raw Player Devices.
  * ToFPMDRawPlayerDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Raw Player Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */
class MITKPMD_EXPORT ToFCameraPMDRawDataCamCubeDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:

  ToFCameraPMDRawDataCamCubeDeviceFactory()
  {
  }
  /*!
   \brief Get the name of the factory, here for the ToFPMDRawDataCamCube.
   */
   std::string GetFactoryName()
   {
       return std::string("PMD RAW Data Camcube Factory ");
   }

   /**
    * @brief GetDeviceNamePrefix Main part of the device name.
    */
   std::string GetDeviceNamePrefix()
   {
       return std::string("PMD Raw Data CamCube 2.0/3.0");
   }

private:
     /*!
   \brief Create an instance of a ToFPMDRawDataDevice.
   */
   ToFCameraDevice::Pointer CreateToFCameraDevice()
   {
     ToFCameraPMDRawDataCamCubeDevice::Pointer device = ToFCameraPMDRawDataCamCubeDevice::New();

      device->SetBoolProperty("HasRGBImage", false);
      device->SetBoolProperty("HasAmplitudeImage", true);
      device->SetBoolProperty("HasIntensityImage", true);

     return device.GetPointer();
   }

   us::ModuleResource GetIntrinsicsResource()
   {
     us::Module* module = us::GetModuleContext()->GetModule();
     return module->GetResource("CalibrationFiles/PMDCamCube3_camera.xml");
   }
};
}
#endif
