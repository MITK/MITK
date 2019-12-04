/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFCameraPMDRawDataCamBoardDeviceFactory_h
#define __mitkToFCameraPMDRawDataCamBoardDeviceFactory_h

#include "mitkPMDModuleExports.h"
#include "mitkToFCameraPMDRawDataCamBoardDevice.h"
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

class MITKPMD_EXPORT ToFCameraPMDRawDataCamBoardDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:

  ToFCameraPMDRawDataCamBoardDeviceFactory()
  {
  }

  /*!
  \brief Defining the Factorie´s Name, here for the RawDataCamBoardDeviceFactory.
  */
  std::string GetFactoryName()
  {
    return std::string("PMD Raw Data CamBoard Factory ");
  }

  std::string GetDeviceNamePrefix()
  {
    return std::string("PMD Raw Data CamBoard Device");
  }

private:
     /*!
   \brief Create an instance of a RawDataCamBoardDeviceFactory.
   */
   ToFCameraDevice::Pointer CreateToFCameraDevice()
   {
     ToFCameraPMDRawDataCamBoardDevice::Pointer device = ToFCameraPMDRawDataCamBoardDevice::New();

     return device.GetPointer();
   }

   us::ModuleResource GetIntrinsicsResource()
   {
     us::Module* module = us::GetModuleContext()->GetModule();
     return module->GetResource("CalibrationFiles/PMDCamBoard_camera.xml");
   }
};
}
#endif
