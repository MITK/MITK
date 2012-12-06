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
#ifndef __mitkToFCameraPMDCamCubeDeviceFactory_h
#define __mitkToFCameraPMDCamCubeDeviceFactory_h

#include "mitkPMDModuleExports.h"
#include "mitkToFCameraPMDCamCubeDevice.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief ToFPMDCamBoardDeviceFactory is an implementation of the factory pattern to generate Cam Cube Devices.
  * ToFPMDCamCubeDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Cam Cube Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_PMDMODULE_EXPORT ToFCameraPMDCamCubeDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
  ToFCameraPMDCamCubeDeviceFactory()
  {
    this->m_DeviceNumber=1;
  }
   /*!
   \brief Defining the Factorie´s Name, here for the ToFPMDCamCube.
   */
   std::string GetFactoryName()
   {
       return std::string("PMD Camcube 2.0/3.0 Factory ");
   }

   std::string GetCurrentDeviceName()
   {
     std::stringstream name;
     if(m_DeviceNumber>1)
     {
       name << "PMD CamCube 2.0/3.0 "<< m_DeviceNumber;
     }
     else
     {
       name << "PMD CamCube 2.0/3.0 ";
     }
     m_DeviceNumber++;
     return name.str();
   }

private:
     /*!
   \brief Create an instance of a ToFPMDCamCubeDevice.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraPMDCamCubeDevice::Pointer device = ToFCameraPMDCamCubeDevice::New();

     return device.GetPointer();
   }
    int m_DeviceNumber;
};
}
#endif
