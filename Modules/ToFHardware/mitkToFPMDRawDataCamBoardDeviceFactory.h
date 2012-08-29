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
#ifndef __mitkToFPMDRawDataCamBoardDeviceFactory_h
#define __mitkToFPMDRawDataCamBoardDeviceFactory_h

#include "mitkToFHardwareExports.h"
#include "mitkToFCameraPMDRawDataCamBoardDevice.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief ToFPMDRawPlayerDeviceFactory is an implementation of the factory pattern to generate Raw Player Devices.
  * ToFPMDRawPlayerDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new Raw Player Devices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_TOFHARDWARE_EXPORT ToFPMDRawDataCamBoardDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
  /*!
   \brief Defining the Factorie´s Name, here for the RawDataCamBoardDeviceFactory.
   */
   std::string GetFactoryName()
   {
       return std::string("PMD RAW Data Cam Board Device Factory");
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
};
}
#endif
