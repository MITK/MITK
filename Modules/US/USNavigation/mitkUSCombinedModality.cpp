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


mitk::USImageSource::Pointer mitk::USCombinedModality::GetUSImageSource()
{
  if (m_UltrasoundDevice.IsNull())
  {
    MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
    mitkThrow() << "UltrasoundDevice must not be null.";
  }

  return m_UltrasoundDevice->GetUSImageSource();
}

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

void mitk::USCombinedModality::UnregisterOnService()
{
  if (m_DeviceState == State_Activated) { this->Deactivate(); }
  if (m_DeviceState == State_Connected) { this->Disconnect(); }

  mitk::USDevice::UnregisterOnService();
}

mitk::AffineTransform3D::Pointer mitk::USCombinedModality::GetCalibration()
{
  return this->GetCalibration(this->GetCurrentDepthValue(), this->GetIdentifierForCurrentProbe());
}

mitk::AffineTransform3D::Pointer mitk::USCombinedModality::GetCalibration(std::string depth)
{
  return this->GetCalibration(depth, this->GetIdentifierForCurrentProbe());
}

mitk::AffineTransform3D::Pointer mitk::USCombinedModality::GetCalibration(std::string depth, std::string probe)
{
  // make sure that there is no '/' which would cause problems for TinyXML
  std::replace(probe.begin(), probe.end(), '/', '-');

  // create identifier for calibration from probe and depth
  std::string calibrationKey = probe + mitk::USCombinedModality::ProbeAndDepthSeperator + depth;

  // find calibration for combination of probe identifier and depth
  std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator calibrationIterator
    = m_Calibrations.find(calibrationKey);

  if (calibrationIterator == m_Calibrations.end()) { return nullptr; }

  return calibrationIterator->second;
}

void mitk::USCombinedModality::SetCalibration(mitk::AffineTransform3D::Pointer calibration)
{
  if (calibration.IsNull())
  {
    MITK_WARN << "Null pointer passed to SetCalibration of mitk::USDevice. Ignoring call.";
    return;
  }

  std::string calibrationKey = this->GetIdentifierForCurrentCalibration();
  if (calibrationKey.empty())
  {
    MITK_WARN << "Could not get a key for the calibration -> Calibration cannot be set.";
    return;
  }

  m_Calibrations[calibrationKey] = calibration;
}

bool mitk::USCombinedModality::RemoveCalibration()
{
  return this->RemoveCalibration(this->GetCurrentDepthValue(), this->GetIdentifierForCurrentProbe());
}

bool mitk::USCombinedModality::RemoveCalibration(std::string depth)
{
  return this->RemoveCalibration(depth, this->GetIdentifierForCurrentProbe());
}

bool mitk::USCombinedModality::RemoveCalibration(std::string depth, std::string probe)
{
  // make sure that there is no '/' which would cause problems for TinyXML
  std::replace(probe.begin(), probe.end(), '/', '-');

  // create identifier for calibration from probe and depth
  std::string calibrationKey = probe + mitk::USCombinedModality::ProbeAndDepthSeperator + depth;

  return m_Calibrations.erase(calibrationKey) > 0;
}

void mitk::USCombinedModality::SetNumberOfSmoothingValues(unsigned int numberOfSmoothingValues)
{
  unsigned int oldNumber = m_NumberOfSmoothingValues;
  m_NumberOfSmoothingValues = numberOfSmoothingValues;

  // if filter should be activated or deactivated
  if ((oldNumber == 0 && numberOfSmoothingValues != 0) ||
    (oldNumber != 0 && numberOfSmoothingValues == 0))
  {
    this->RebuildFilterPipeline();
  }
  m_SmoothingFilter->SetNumerOfValues(numberOfSmoothingValues);
}

void mitk::USCombinedModality::SetDelayCount(unsigned int delayCount)
{
  unsigned int oldCount = m_DelayCount;
  m_DelayCount = delayCount;

  // if filter should be activated or deactivated
  if ((oldCount == 0 && delayCount != 0) ||
    (oldCount != 0 && delayCount == 0))
  {
    this->RebuildFilterPipeline();
  }
  m_DelayFilter->SetDelay(delayCount);
}

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

mitk::NavigationDataSource::Pointer mitk::USCombinedModality::GetNavigationDataSource()
{
  return m_LastFilter.GetPointer();
}

bool mitk::USCombinedModality::GetIsCalibratedForCurrentStatus()
{
  return m_Calibrations.find(this->GetIdentifierForCurrentCalibration()) != m_Calibrations.end();
}

bool mitk::USCombinedModality::GetContainsAtLeastOneCalibration()
{
  return !m_Calibrations.empty();
}

void mitk::USCombinedModality::GenerateData()
{
  if (m_UltrasoundDevice->GetIsFreezed()) { return; } //if the image is freezed: do nothing
  //get next image from ultrasound image source
  //FOR LATER: Be aware if the for loop behaves correct, if the UltrasoundDevice has more than 1 output.
  int i = 0;
  mitk::Image::Pointer image = m_UltrasoundDevice->GetUSImageSource()->GetNextImage().at(i);
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
