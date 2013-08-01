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
#include <mitkModuleContext.h>
#include <usGetModuleContext.h>
#include <mitkModule.h>
#include <mitkModuleResource.h>
#include <mitkModuleResourceStream.h>

namespace mitk
{
  /**
  * \brief ToFPMDRawPlayerDeviceFactory is an implementation of the factory pattern to generate Raw Player Devices.
  * ToFPMDRawPlayerDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Raw Player Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_PMDMODULE_EXPORT ToFCameraPMDRawDataCamBoardDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:

  ToFCameraPMDRawDataCamBoardDeviceFactory()
  {
    this->m_DeviceNumber=0;
  }

  /*!
   \brief Defining the Factorie´s Name, here for the RawDataCamBoardDeviceFactory.
   */
   std::string GetFactoryName()
   {
       return std::string("PMD Raw Data CamBoard Factory ");
   }

   std::string GetCurrentDeviceName()
   {
     std::stringstream name;
     name<<"PMD Raw Data CamBoard Device " << m_DeviceNumber++;
     return name.str ();
   }

private:
     /*!
   \brief Create an instance of a RawDataCamBoardDeviceFactory.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraPMDRawDataCamBoardDevice::Pointer device = ToFCameraPMDRawDataCamBoardDevice::New();

     return device.GetPointer();
   }

   ModuleResource GetIntrinsicsResource()
   {
     Module* module = GetModuleContext()->GetModule();
     return module->GetResource("CalibrationFiles/PMDCamBoard_camera.xml");
   }

   int m_DeviceNumber;
};
}
#endif
