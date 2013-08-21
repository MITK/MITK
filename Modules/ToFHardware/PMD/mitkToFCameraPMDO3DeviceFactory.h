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
#ifndef __mitkToFCameraPMDO3DeviceFactory_h
#define __mitkToFCameraPMDO3DeviceFactory_h

#include "mitkPMDModuleExports.h"
#include "mitkToFCameraPMDO3Device.h"
#include "mitkAbstractToFDeviceFactory.h"
#include <mitkCameraIntrinsics.h>
#include <mitkCameraIntrinsicsProperty.h>
#include <mitkToFConfig.h>


namespace mitk
{
  /**
  * \brief ToFCameraPMDO3DeviceFactory is an implementation of the factory pattern to generate Do3 Devices.
  * ToFPMDCamCubeDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Cam Cube Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_PMDMODULE_EXPORT ToFCameraPMDO3DeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
    ToFCameraPMDO3DeviceFactory()
    {
      this->m_DeviceNumber =1;
    }

   /*!
   \brief Defining the Factorie�s Name, here for the ToFPMDO3Device
   */
   std::string GetFactoryName()
   {
       return std::string("PMD O3D Factory");
   }
   std::string GetCurrentDeviceName()
   {
     std::stringstream name;
     if(m_DeviceNumber>1)
     {
       name << "PMD O3 "<< m_DeviceNumber;
     }
     else
     {
       name << "PMD O3";
     }
     m_DeviceNumber++;
     return name.str();
   }

private:
     /*!
   \brief Create an instance of a ToFPMDO3DeviceFactory.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraPMDO3Device::Pointer device = ToFCameraPMDO3Device::New();

     //-------------------------Intrinsics for 03 are missing-----------------------------------
     //Set default camera intrinsics for the CamBoard-camera.
     mitk::CameraIntrinsics::Pointer cameraIntrinsics = mitk::CameraIntrinsics::New();
     std::string pathToDefaulCalibrationFile(MITK_TOF_DATA_DIR);

     pathToDefaulCalibrationFile.append("/CalibrationFiles/Default_Parameters.xml");
     cameraIntrinsics->FromXMLFile(pathToDefaulCalibrationFile);
     device->SetProperty("CameraIntrinsics", mitk::CameraIntrinsicsProperty::New(cameraIntrinsics));

     //------------------------------------------------------------------------------------------
     device->SetBoolProperty("HasRGBImage", false);
     device->SetBoolProperty("HasAmplitudeImage", true);
     device->SetBoolProperty("HasIntensityImage", true);

     return device.GetPointer();
   }
   int m_DeviceNumber;
};
}
#endif
