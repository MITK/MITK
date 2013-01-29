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
#ifndef __mitkToFCameraMITKPlayerDeviceFactory_h
#define __mitkToFCameraMITKPlayerDeviceFactory_h

#include "mitkToFHardwareExports.h"
#include "mitkToFCameraMITKPlayerDevice.h"
#include "mitkAbstractToFDeviceFactory.h"

namespace mitk
{
  /**
  * \brief ToFPlayerDeviceFactory is an implementation of the factory pattern to generate ToFPlayer devices.
  * ToFPlayerDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new ToFPlayerDevices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITK_TOFHARDWARE_EXPORT ToFCameraMITKPlayerDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

public:


  ToFCameraMITKPlayerDeviceFactory()
  {
   m_DeviceNumber = 1;
  }

     /*!
   \brief Defining the Factorie´s Name, here for the ToFPlayer.
   */
   std::string GetFactoryName()
   {
       return std::string("MITK Player Factory");
   }

    std::string GetCurrentDeviceName()
    {
      std::stringstream name;
      if(m_DeviceNumber>1)
      {
        name << "MITK Player "<< m_DeviceNumber;
      }
      else
      {
        name << "MITK Player";
      }
      m_DeviceNumber++;
      return name.str();
    }

private:
   /*!
   \brief Create an instance of a ToFPlayerDevice.
   */
   ToFCameraDevice::Pointer createToFCameraDevice()
   {
     ToFCameraMITKPlayerDevice::Pointer device = ToFCameraMITKPlayerDevice::New();

     return device.GetPointer();
   }

   int m_DeviceNumber;

};
}
#endif
