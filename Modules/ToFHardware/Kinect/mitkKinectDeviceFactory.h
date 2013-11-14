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

#include "mitkKinectModuleExports.h"
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
  class MITK_KINECTMODULE_EXPORT KinectDeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

  public:

    /**
     * @brief KinectDeviceFactory Default contructor.
     * This factory internally counts all kinect devices starting at 1.
     */
    KinectDeviceFactory()
    {
      this->m_DeviceNumber = 1;
    }
    /*!
    \brief Get the name of the factory, here for the Kinect.
    */
    std::string GetFactoryName()
    {
      return std::string("Kinect Factory");
    }
    /**
     * @brief GetCurrentDeviceName Get the name of the current kinect.
     * First device is named "Kinect", second "Kinect 2" and so on.
     * @return Human readable name as string.
     */
    std::string GetCurrentDeviceName()
    {
      std::stringstream name;
      if (m_DeviceNumber>1)
      {
        name << "Kinect " << m_DeviceNumber;
      }
      else
      {
        name << "Kinect ";
      }
      m_DeviceNumber++;
      return name.str();
    }

  private:

    /*!
    \brief Create an instance of a KinectDevice.
    This method sets default properties for the kinect device.
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

    /**
     * @brief m_DeviceNumber Member for counting of devices.
     */
    int m_DeviceNumber;
  };
}
#endif
