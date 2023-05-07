/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFCameraMITKPlayerDeviceFactory_h
#define mitkToFCameraMITKPlayerDeviceFactory_h

#include <MitkToFHardwareExports.h>
#include "mitkToFCameraMITKPlayerDevice.h"
#include "mitkAbstractToFDeviceFactory.h"
#include <mitkCameraIntrinsics.h>
#include <mitkCameraIntrinsicsProperty.h>
#include <mitkToFConfig.h>

namespace mitk
{
  /**
  * \brief ToFPlayerDeviceFactory is an implementation of the factory pattern to generate ToFPlayer devices.
  * ToFPlayerDeviceFactory inherits from AbstractToFDeviceFactory which is a MicroService interface.
  * This offers users the oppertunity to generate new ToFPlayerDevices via a global instance of this factory.
  * @ingroup ToFHardware
  */

class MITKTOFHARDWARE_EXPORT ToFCameraMITKPlayerDeviceFactory : public AbstractToFDeviceFactory {

public:


  ToFCameraMITKPlayerDeviceFactory()
  {
  }

    /*!
  \brief Defining the Factorie's Name, here for the ToFPlayer.
  */
  std::string GetFactoryName() override
  {
    return std::string("MITK Player Factory");
  }

  std::string GetDeviceNamePrefix() override
  {
    return std::string("MITK Player");
  }

private:
   /*!
   \brief Create an instance of a ToFPlayerDevice.
   */
   ToFCameraDevice::Pointer CreateToFCameraDevice() override
   {
     ToFCameraMITKPlayerDevice::Pointer device = ToFCameraMITKPlayerDevice::New();

////-------------------------If no Intrinsics are specified------------------------------
//      //Set default camera intrinsics for the MITK-Player.
//      mitk::CameraIntrinsics::Pointer cameraIntrinsics = mitk::CameraIntrinsics::New();
//      std::string pathToDefaulCalibrationFile(MITK_TOF_DATA_DIR);
//
//      pathToDefaulCalibrationFile.append("/CalibrationFiles/Default_Parameters.xml");
//      cameraIntrinsics->FromXMLFile(pathToDefaulCalibrationFile);
//      device->SetProperty("CameraIntrinsics", mitk::CameraIntrinsicsProperty::New(cameraIntrinsics));
//
////------------------------------------------------------------------------------------------

     return device.GetPointer();
   }
};
}
#endif
