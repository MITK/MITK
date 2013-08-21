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
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraPMDCamBoardDevice::Pointer device = ToFCameraPMDCamBoardDevice::New();

      //Set default camera intrinsics for the CamBoard-camera.
      mitk::CameraIntrinsics::Pointer cameraIntrinsics = mitk::CameraIntrinsics::New();
      std::string pathToDefaulCalibrationFile(MITK_TOF_DATA_DIR);

      pathToDefaulCalibrationFile.append("/CalibrationFiles/PMDCamBoard_camera.xml");
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
