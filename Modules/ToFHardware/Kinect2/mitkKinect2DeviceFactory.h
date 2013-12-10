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
#ifndef __mitkKinect2DeviceFactory_h
#define __mitkKinect2DeviceFactory_h

#include "mitkKinect2ModuleExports.h"
#include "mitkKinect2Device.h"
#include "mitkAbstractToFDeviceFactory.h"
#include <mitkCameraIntrinsics.h>
#include <mitkCameraIntrinsicsProperty.h>

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
  * \brief KinectDeviceFactory is an implementation of the factory pattern to generate Microsoft Kinect devices.
  * KinectDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new KinectDevices via a global instance of this factory.
  * @ingroup ToFHardware
  */
  class MITK_KINECT2MODULE_EXPORT Kinect2DeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

  public:

    Kinect2DeviceFactory()
    {
    }

    /**
     * @brief GetFactoryName Get the name of the factory.
     * @return Name as human readable string.
     */
    std::string GetFactoryName()
    {
      return std::string("Kinect 2 Factory");
    }
    /**
     * @brief GetDeviceNamePrefix Main part of a device name.
     */
    std::string GetDeviceNamePrefix()
    {
      return std::string("Kinect 2");
    }

  private:

    /*!
    \brief Create an instance of a KinectDevice.
    */
    ToFCameraDevice::Pointer CreateToFCameraDevice()
    {
      Kinect2Device::Pointer device = Kinect2Device::New();

      device->SetBoolProperty("HasRGBImage", true);
      device->SetBoolProperty("HasAmplitudeImage", false);
      device->SetBoolProperty("HasIntensityImage", false);
      device->SetBoolProperty("KinectReconstructionMode", true);
      device->SetBoolProperty("RGBImageHasDifferentResolution", true);

      return device.GetPointer();
    }

    us::ModuleResource GetIntrinsicsResource()
    {
      us::Module* module = us::GetModuleContext()->GetModule();
      return module->GetResource("CalibrationFiles/Kinect_RGB_camera.xml");
    }
  };
}
#endif
