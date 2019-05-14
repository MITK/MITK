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

#include "mitkImageReadAccessor.h"
#include "mitkTrackingDeviceSource.h"
#include "mitkUSCombinedModality.h"
#include <mitkNavigationDataDelayFilter.h>
#include <mitkNavigationDataSmoothingFilter.h>

// US Control Interfaces
#include "mitkUSControlInterfaceBMode.h"
#include "mitkUSControlInterfaceDoppler.h"
#include "mitkUSControlInterfaceProbes.h"

mitk::USCombinedModality::USCombinedModality(USDevice::Pointer usDevice,
                                             NavigationDataSource::Pointer trackingDevice,
                                             bool trackedUltrasoundActive)
  : AbstractUltrasoundTrackerDevice(usDevice, trackingDevice, trackedUltrasoundActive)
{
}
mitk::AffineTransform3D::Pointer mitk::USCombinedModality::GetUSPlaneTransform()
{
  return this->GetCalibration();
}

mitk::USCombinedModality::~USCombinedModality() {}

void mitk::USCombinedModality::GenerateData()
{
  if (this->GetIsFreezed())
  {
    return;
  } // if the image is freezed: do nothing

  // get next image from ultrasound image source
  // FOR LATER: Be aware if the for loop behaves correct, if the UltrasoundDevice has more than 1 output.
  m_UltrasoundDevice->Update();
  mitk::Image::Pointer image = m_UltrasoundDevice->GetOutput(0);
  if (image.IsNull() || !image->IsInitialized()) // check the image
  {
    MITK_WARN << "Invalid image in USCombinedModality, aborting!";
    return;
  }

  // get output and initialize it if it wasn't initialized before
  mitk::Image::Pointer output = this->GetOutput();
  if (!output->IsInitialized())
  {
    output->Initialize(image);
  }

  // now update image data
  mitk::ImageReadAccessor inputReadAccessor(image, image->GetSliceData(0, 0, 0));
  output->SetSlice(inputReadAccessor.GetData());                         // copy image data
  output->GetGeometry()->SetSpacing(image->GetGeometry()->GetSpacing()); // copy spacing because this might also change

  // and update calibration (= transformation of the image)
  std::string calibrationKey = this->GetIdentifierForCurrentCalibration();
  if (!calibrationKey.empty())
  {
    std::map<std::string, mitk::AffineTransform3D::Pointer>::iterator calibrationIterator =
      m_Calibrations.find(calibrationKey);
    if (calibrationIterator != m_Calibrations.end())
    {
      // transform image according to callibration if one is set
      // for current configuration of probe and depth
      this->GetOutput()->GetGeometry()->SetIndexToWorldTransform(calibrationIterator->second);
    }
  }
}

void mitk::USCombinedModality::OnFreeze(bool freeze)
{
  mitk::TrackingDeviceSource::Pointer trackingDeviceSource = dynamic_cast<mitk::TrackingDeviceSource*>(m_TrackingDeviceDataSource.GetPointer());
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
