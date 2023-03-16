/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkKinectDeviceFactory_h
#define mitkKinectDeviceFactory_h

#include <MitkKinectExports.h>
#include "mitkKinectDevice.h"
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
  class MITKKINECT_EXPORT KinectDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

  public:

    /**
     * @brief KinectDeviceFactory Default contructor.
     * This factory internally counts all kinect devices starting at 1.
     */
    KinectDeviceFactory()
    {
    }
    /*!
    \brief Get the name of the factory, here for the Kinect.
    */
    std::string GetFactoryName()
    {
      return std::string("Kinect Factory");
    }

    /**
     * @brief GetDeviceNamePrefix Main part of a device name.
     */
    std::string GetDeviceNamePrefix()
    {
      return std::string("Kinect");
    }

  private:

    /*!
    \brief Create an instance of a KinectDevice.
    */
    ToFCameraDevice::Pointer CreateToFCameraDevice()
    {
      KinectDevice::Pointer device = KinectDevice::New();

      device->SetBoolProperty("HasRGBImage", true);
      device->SetBoolProperty("HasAmplitudeImage", false);
      device->SetBoolProperty("HasIntensityImage", false);
      device->SetBoolProperty("KinectReconstructionMode", true);

      return device.GetPointer();
    }

    /**
     * @brief GetIntrinsicsResource Get the resource of the
     * default camera intrinsics for a kinect. If you want to
     * use your own camera intrinsics, just overwrit the
     * CameraIntrinsicsProperty of your device.
     * @return A resource path to the camera .xml file.
     */
    us::ModuleResource GetIntrinsicsResource()
    {
      us::Module* module = us::GetModuleContext()->GetModule();
      return module->GetResource("CalibrationFiles/Kinect_RGB_camera.xml");
    }
  };
}
#endif
