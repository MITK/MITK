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
#include "mitkNavigationDataDisplacementFilter.h"
#include "mitkTrackingDeviceSource.h"




mitk::TrackedUltrasound::TrackedUltrasound( USDevice::Pointer usDevice,
                                              NavigationDataSource::Pointer trackingDevice,
                                              bool trackedUltrasoundActive )
  : AbstractUltrasoundTrackerDevice( usDevice, trackingDevice, trackedUltrasoundActive )
{
}

mitk::TrackedUltrasound::~TrackedUltrasound()
{
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
      m_DisplacementFilter->SetTransformation(calibrationIterator->second);
    }
  }
}
