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

#include "mitkUSCombinedModality.h"
//#include "mitkNavigationDataSource.h"
#include "mitkImageReadAccessor.h"
#include <mitkNavigationDataSmoothingFilter.h>
#include <mitkNavigationDataDelayFilter.h>
#include "mitkTrackingDeviceSource.h"

// US Control Interfaces
#include "mitkUSControlInterfaceProbes.h"
#include "mitkUSControlInterfaceBMode.h"
#include "mitkUSControlInterfaceDoppler.h"




mitk::USCombinedModality::USCombinedModality( USDevice::Pointer usDevice,
                                              NavigationDataSource::Pointer trackingDevice,
                                              bool trackedUltrasoundActive )
  : AbstractUltrasoundTrackerDevice( usDevice, trackingDevice, trackedUltrasoundActive )
{

}

mitk::USCombinedModality::~USCombinedModality()
{

}
/*
void mitk::USCombinedModality::RegisterAsMicroservice()
{
  //Get Context
  us::ModuleContext* context = us::GetModuleContext();

  //Define ServiceProps
  //us::ServiceProperties props;
  mitk::UIDGenerator uidGen =
    mitk::UIDGenerator("org.mitk.services.AbstractUltrasoundTrackerDevice", 16);
  m_ServiceProperties[US_PROPKEY_ID] = uidGen.GetUID();
  m_ServiceProperties[US_PROPKEY_DEVICENAME] = m_UltrasoundDevice->GetName();
  m_ServiceProperties[US_PROPKEY_CLASS] = mitk::AbstractUltrasoundTrackerDevice::DeviceClassIdentifier;

  m_ServiceRegistration = context->RegisterService(this, m_ServiceProperties);
}*/



mitk::USAbstractControlInterface::Pointer mitk::USCombinedModality::GetControlInterfaceCustom()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceCustom();
}

mitk::USControlInterfaceBMode::Pointer mitk::USCombinedModality::GetControlInterfaceBMode()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceBMode();
}

mitk::USControlInterfaceProbes::Pointer mitk::USCombinedModality::GetControlInterfaceProbes()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceProbes();
}

mitk::USControlInterfaceDoppler::Pointer mitk::USCombinedModality::GetControlInterfaceDoppler()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetControlInterfaceDoppler();
}


void mitk::USCombinedModality::GenerateData()
{
  if (m_UltrasoundDevice->GetIsFreezed()) { return; } //if the image is freezed: do nothing

  //get next image from ultrasound image source
  mitk::Image::Pointer image = m_UltrasoundDevice->GetOutput(); //GetUSImageSource()->GetNextImage();

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


//======================================================================================================================
/**
bool mitk::USCombinedModality::OnInitialization()
{
if (m_UltrasoundDevice.IsNull())
{
MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
mitkThrow() << "UltrasoundDevice must not be null.";
}

if (m_UltrasoundDevice->GetDeviceState() < mitk::USDevice::State_Initialized)
{
return m_UltrasoundDevice->Initialize();
}
else
{
return true;
}
}

bool mitk::USCombinedModality::OnConnection()
{
if (m_UltrasoundDevice.IsNull())
{
MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
mitkThrow() << "UltrasoundDevice must not be null.";
}

// connect ultrasound device only if it is not already connected
if (m_UltrasoundDevice->GetDeviceState() >= mitk::USDevice::State_Connected)
{
return true;
}
else
{
return m_UltrasoundDevice->Connect();
}
}

bool mitk::USCombinedModality::OnDisconnection()
{
if (m_UltrasoundDevice.IsNull())
{
MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
mitkThrow() << "UltrasoundDevice must not be null.";
}

return m_UltrasoundDevice->Disconnect();
}

bool mitk::USCombinedModality::OnActivation()
{
if (m_UltrasoundDevice.IsNull())
{
MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
mitkThrow() << "UltrasoundDevice must not be null.";
}

mitk::TrackingDeviceSource::Pointer trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_TrackingDevice.GetPointer());
if (trackingDeviceSource.IsNull())
{
MITK_WARN("USCombinedModality")("USDevice") << "Cannot start tracking as TrackingDeviceSource is null.";
}
trackingDeviceSource->StartTracking();

// activate ultrasound device only if it is not already activated
if (m_UltrasoundDevice->GetDeviceState() >= mitk::USDevice::State_Activated)
{
return true;
}
else
{
return m_UltrasoundDevice->Activate();
}
}

bool mitk::USCombinedModality::OnDeactivation()
{
if (m_UltrasoundDevice.IsNull())
{
MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
mitkThrow() << "UltrasoundDevice must not be null.";
}

mitk::TrackingDeviceSource::Pointer trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_TrackingDevice.GetPointer());
if (trackingDeviceSource.IsNull())
{
MITK_WARN("USCombinedModality")("USDevice") << "Cannot stop tracking as TrackingDeviceSource is null.";
}
trackingDeviceSource->StopTracking();

m_UltrasoundDevice->Deactivate();

return m_UltrasoundDevice->GetIsConnected();
}

void mitk::USCombinedModality::OnFreeze(bool freeze)
{
mitk::TrackingDeviceSource::Pointer trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_TrackingDevice.GetPointer());
if (trackingDeviceSource.IsNull())
{
MITK_WARN("USCombinedModality")("USDevice") << "Cannot freeze tracking.";
}
else
{
if (freeze) { trackingDeviceSource->Freeze(); }
else { trackingDeviceSource->UnFreeze(); }
}

if (m_UltrasoundDevice.IsNull())
{
MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
mitkThrow() << "UltrasoundDevice must not be null.";
}
m_UltrasoundDevice->SetIsFreezed(freeze);
}
*/
