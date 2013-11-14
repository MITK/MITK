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

#include "mitkPMDModuleExports.h"
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
class MITK_PMDMODULE_EXPORT ToFCameraPMDRawDataCamCubeDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:

  /**
   * @brief ToFCameraPMDRawDataCamCubeDeviceFactory Default contructor.
   * This factory internally counts all raw data Cam Cube devices starting at 1.
   */
  ToFCameraPMDRawDataCamCubeDeviceFactory()
  {
    this->m_DeviceNumber = 1;
  }
  /*!
   \brief Get the name of the factory, here for the ToFPMDRawDataCamCube.
   */
  std::string GetFactoryName()
  {
    return std::string("PMD RAW Data Camcube Factory ");
  }

  /**
    * @brief GetCurrentDeviceName Get the name of the current raw data Cam Cube.
    * First device is named "PMD Raw Data CamCube 2.0/3.0", second "PMD Raw Data CamCube 2.0/3.0 2" and so on.
    * @return Human readable name as string.
    */
  std::string GetCurrentDeviceName()
  {
    std::stringstream name;
    if(m_DeviceNumber>1)
    {
      name << "PMD Raw Data CamCube 2.0/3.0 "<< m_DeviceNumber;
    }
    else
    {
      name << "PMD Raw Data CamCube 2.0/3.0";
    }
    m_DeviceNumber++;
    return name.str();
  }

private:
  /*!
     \brief Create an instance of a ToFCameraPMDRawDataCamCubeDevice.
     Sets default properties for a PMD raw data Cam Cube.
     */
  ToFCameraDevice::Pointer CreateToFCameraDevice()
  {
    ToFCameraPMDRawDataCamCubeDevice::Pointer device = ToFCameraPMDRawDataCamCubeDevice::New();

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
