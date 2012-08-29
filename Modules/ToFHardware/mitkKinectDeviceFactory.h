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
#ifndef __mitkKinectDeviceFactory_h
#define __mitkKinectDeviceFactory_h

#include "mitkToFHardwareExports.h"
#include "mitkKinectDevice.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief KinectDeviceFactory is an implementation of the factory pattern to generate Microsoft Kinect devices.
  * KinectDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new KinectDevices via a global instance of this factory.
  * @ingroup ToFHardware
  */
class MITK_TOFHARDWARE_EXPORT KinectDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:
     /*!
   \brief Defining the Factorie´s Name, here for the Kinect.
   */
   std::string GetFactoryName()
   {
       return std::string("Microsoft Kinect Device Factory");
   }

private:

   /*!
   \brief Create an instance of a KinectDevice.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     KinectDevice::Pointer device = KinectDevice::New();

     return device.GetPointer();
   }
};
}
#endif
