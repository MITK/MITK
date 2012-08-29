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
#ifndef __mitkToFPMDPlayerDeviceFactory_h
#define __mitkToFPMDPlayerDeviceFactory_h

#include "mitkToFHardwareExports.h"
#include "mitkToFCameraPMDPlayerDevice.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief ToFPMDPlayerDeviceFactory is an implementation of the factory pattern to generate PMD Player Devices.
  * ToFPMDPlayerDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new PMD Player Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_TOFHARDWARE_EXPORT ToFPMDPlayerDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
  /*!
   \brief Defining the Factorie´s Name, here for the ToFPMDPlayer.
   */
   std::string GetFactoryName()
   {
       return std::string("PMD Player Device Factory");
   }
private:
     /*!
   \brief Create an instance of a ToFPMDPlayerDevice.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraPMDPlayerDevice::Pointer device = ToFCameraPMDPlayerDevice::New();

     return device.GetPointer();
   }


};
}
#endif
