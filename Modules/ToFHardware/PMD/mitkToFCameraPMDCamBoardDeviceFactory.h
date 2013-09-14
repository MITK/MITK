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
#ifndef __mitkToFCameraPMDCamBoardDeviceFactory_h
#define __mitkToFCameraPMDCamBoardDeviceFactory_h

#include "mitkPMDModuleExports.h"
#include "mitkToFCameraPMDCamBoardDevice.h"
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
  * \brief ToFPMDCamBoardDeviceFactory is an implementation of the factory pattern to generate CamBoard devices.
  * ToFPMDCamBoardDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new CamBoard Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_PMDMODULE_EXPORT ToFCameraPMDCamBoardDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
  ToFCameraPMDCamBoardDeviceFactory()
  {
    this->m_DeviceNumber=1;
  }
  /*!
   \brief Defining the Factorie�s Name, here for the ToFPMDCamBoard.
   */
   std::string GetFactoryName()
   {
       return std::string("PMD CamBoard Factory ");
   }

    std::string GetCurrentDeviceName()
    {
     std::stringstream name;
     if(m_DeviceNumber>1)
     {
       name << "PMD CamBoard "<< m_DeviceNumber;
     }
     else
     {
       name << "PMD CamBoard";
     }
     m_DeviceNumber++;
     return name.str();
    }

private:

     /*!
   \brief Create an instance of a ToFPMDCamBoardDevice.
   */
   ToFCameraDevice::Pointer CreateToFCameraDevice()
   {
     ToFCameraPMDCamBoardDevice::Pointer device = ToFCameraPMDCamBoardDevice::New();

      device->SetBoolProperty("HasRGBImage", false);
      device->SetBoolProperty("HasAmplitudeImage", true);
      device->SetBoolProperty("HasIntensityImage", true);

     return device.GetPointer();
   }

   us::ModuleResource GetIntrinsicsResource()
   {
     us::Module* module = us::GetModuleContext()->GetModule();
     return module->GetResource("CalibrationFiles/PMDCamBoard_camera.xml");
   }


   int m_DeviceNumber;
};
}
#endif
