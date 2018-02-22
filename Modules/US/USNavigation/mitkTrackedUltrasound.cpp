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

#include "mitkTrackedUltrasound.h"
#include "mitkImageReadAccessor.h"
#include <mitkNavigationDataSmoothingFilter.h>
#include <mitkNavigationDataDelayFilter.h>
#include "mitkTrackingDeviceSource.h"

// US Control Interfaces
#include "mitkUSControlInterfaceProbes.h"
#include "mitkUSControlInterfaceBMode.h"
#include "mitkUSControlInterfaceDoppler.h"


mitk::TrackedUltrasound::TrackedUltrasound( USDevice::Pointer usDevice,
                                              NavigationDataSource::Pointer trackingDevice,
                                              bool trackedUltrasoundActive )
  : AbstractUltrasoundTrackerDevice( usDevice, trackingDevice, trackedUltrasoundActive )
{

}

mitk::TrackedUltrasound::~TrackedUltrasound()
{
  /*if (m_ServiceRegistration != nullptr)
  {
    m_ServiceRegistration.Unregister();
  }
  m_ServiceRegistration = 0;*/
}

/*void mitk::TrackedUltrasound::UnregisterOnService()
{
  if (m_UltrasoundDevice->GetDeviceState() == USDevice::State_Activated)
  {
    m_UltrasoundDevice->Deactivate();
  }
  if (m_UltrasoundDevice->GetDeviceState() == USDevice::State_Connected)
  {
    m_UltrasoundDevice->Disconnect();
  }

  if (m_ServiceRegistration != nullptr)
    m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}*/

/*void mitk::TrackedUltrasound::RegisterAsMicroservice()
{
  //Get Context
  us::ModuleContext* context = us::GetModuleContext();

  //Define ServiceProps
  //us::ServiceProperties props;
  mitk::UIDGenerator uidGen =
    mitk::UIDGenerator("org.mitk.services.AbstractUltrasoundTrackerDevice", 16);
  m_ServiceProperties[US_PROPKEY_ID] = uidGen.GetUID();
  m_ServiceProperties[US_PROPKEY_DEVICENAME] = m_UltrasoundDevice->GetName();
  m_ServiceProperties[US_PROPKEY_CLASS] = mitk::TrackedUltrasound::DeviceClassIdentifier;

  m_ServiceRegistration = context->RegisterService(this, m_ServiceProperties);
}*/

mitk::USAbstractControlInterface::Pointer mitk::TrackedUltrasound::GetControlInterfaceCustom()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceCustom();
}

mitk::USControlInterfaceBMode::Pointer mitk::TrackedUltrasound::GetControlInterfaceBMode()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceBMode();
}

mitk::USControlInterfaceProbes::Pointer mitk::TrackedUltrasound::GetControlInterfaceProbes()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceProbes();
}

mitk::USControlInterfaceDoppler::Pointer mitk::TrackedUltrasound::GetControlInterfaceDoppler()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceDoppler();
}


void mitk::TrackedUltrasound::GenerateData()
{
  if (m_UltrasoundDevice->GetIsFreezed()) { return; } //if the image is freezed: do nothing

  //get next image from ultrasound image source
  mitk::Image::Pointer image = m_UltrasoundDevice->GetUSImageSource()->GetNextImage();

  if (image.IsNull() || !image->IsInitialized()) //check the image
  {
    MITK_WARN << "Invalid image in USCombinedModality, aborting!";
    return;
  }

  //get output and initialize it if it wasn't initialized before
  mitk::Image::Pointer output = this->GetOutput();
  if (!output->IsInitialized()) { output->Initialize(image); }

  //now update image data
  mitk::ImageReadAccessor inputReadAccessor(image, image->GetSliceData(0, 0, 0));
  output->SetSlice(inputReadAccessor.GetData()); //copy image data
  output->GetGeometry()->SetSpacing(image->GetGeometry()->GetSpacing()); //copy spacing because this might also change

  //and update calibration (= transformation of the image)
  std::string calibrationKey = this->GetIdentifierForCurrentCalibration();
  if (!calibrationKey.empty())
  {
    std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator calibrationIterator
      = m_Calibrations.find(calibrationKey);
    if (calibrationIterator != m_Calibrations.end())
    {
      // transform image according to callibration if one is set
      // for current configuration of probe and depth
      this->GetOutput()->GetGeometry()->SetIndexToWorldTransform(calibrationIterator->second);
    }
  }
}
