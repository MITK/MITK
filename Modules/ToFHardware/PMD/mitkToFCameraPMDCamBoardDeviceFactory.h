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

#include "mitkPMDCamCubeModuleExports.h"
#include "mitkToFCameraPMDCamBoardDevice.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief ToFPMDCamBoardDeviceFactory is an implementation of the factory pattern to generate CamBoard devices.
  * ToFPMDCamBoardDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new CamBoard Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_PMDCAMCUBEMODULE_EXPORT ToFCameraPMDCamBoardDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
  /*!
   \brief Defining the Factorie´s Name, here for the ToFPMDCamBoard.
   */
   std::string GetFactoryName()
   {
       return std::string("PMD CamBoard Factory");
   }

private:

     /*!
   \brief Create an instance of a ToFPMDCamBoardDevice.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraPMDCamBoardDevice::Pointer device = ToFCameraPMDCamBoardDevice::New();

     return device.GetPointer();
   }
};
}
#endif
