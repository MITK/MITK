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
#ifndef __mitkKinectV2DeviceFactory_h
#define __mitkKinectV2DeviceFactory_h

#include <MitkKinectV2Exports.h>
#include "mitkKinectV2Device.h"
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
  * \brief KinectDeviceFactory is an implementation of the factory pattern to generate Microsoft Kinect V2 devices.
  * KinectDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new KinectDevices via a global instance of this factory.
  * @ingroup ToFHardware
  */
  class MITK_KINECTV2MODULE_EXPORT KinectV2DeviceFactory : public itk::LightObject, public AbstractToFDeviceFactory {

  public:

    KinectV2DeviceFactory()
    {
    }

    /**
     * @brief GetFactoryName Get the name of the factory.
     * @return Name as human readable string.
     */
    std::string GetFactoryName()
    {
      return std::string("Kinect V2 Factory");
    }
    /**
     * @brief GetDeviceNamePrefix Main part of a device name.
     *
     * The string for the kinect 2 is "Kinect V2" on purpose,
     * to distinguish a "Kinect V2" device from a secondary
     * connected "Kinect 2" (in case you have connected 2x
     * "Kinect" and 1x Kinect V2). This case is rare, but
     * we should be able to distinguish between cameras.
     */
    std::string GetDeviceNamePrefix()
    {
      return std::string("Kinect V2");
    }

  private:

    /*!
    \brief Create an instance of a KinectDevice.
    */
    ToFCameraDevice::Pointer CreateToFCameraDevice()
    {
      KinectV2Device::Pointer device = KinectV2Device::New();

      device->SetBoolProperty("HasRGBImage", true);
      device->SetBoolProperty("HasAmplitudeImage", true);
      device->SetBoolProperty("HasIntensityImage", false);
      device->SetBoolProperty("KinectReconstructionMode", true);
      device->SetBoolProperty("RGBImageHasDifferentResolution", true);
      device->SetBoolProperty("HasSurface", true);
      device->SetBoolProperty("HasTextureCoordinates", true);
      device->SetBoolProperty("GenerateTriangularMesh", false);

      device->SetFloatProperty("TriangulationThreshold", 0.0);

      return device.GetPointer();
    }

    us::ModuleResource GetIntrinsicsResource()
    {
      us::Module* module = us::GetModuleContext()->GetModule();
      return module->GetResource("CalibrationFiles/KinectV2_IR_camera.xml");
    }
  };
}
#endif
