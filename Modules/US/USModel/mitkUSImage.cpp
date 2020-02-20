/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSImage.h"
#include <mitkStringProperty.h>
#include <mitkProperties.h>
#include <mitkImageReadAccessor.h>


mitk::USImage::USImage() : mitk::Image()
{
  this->SetMetadata(mitk::USImageMetadata::New());
}

mitk::USImage::USImage(mitk::Image::Pointer image) : mitk::Image()
{
  try
  {
  this->Initialize(image);
  mitk::ImageReadAccessor imgA(image, image->GetVolumeData(0));
  this->SetVolume(imgA.GetData());
  }
  catch(mitk::Exception& e)
  {
  mitkReThrow(e) << "Cannot access image data while constructing US image";
  }
}

mitk::USImage::~USImage()
{

}

mitk::USImageMetadata::Pointer mitk::USImage::GetMetadata() const {
  mitk::USImageMetadata::Pointer result = mitk::USImageMetadata::New();

  result->SetDeviceManufacturer(this->GetProperty(mitk::USImageMetadata::PROP_DEV_MANUFACTURER)->GetValueAsString());
  result->SetDeviceModel(       this->GetProperty(mitk::USImageMetadata::PROP_DEV_MODEL)->GetValueAsString());
  result->SetDeviceComment(     this->GetProperty(mitk::USImageMetadata::PROP_DEV_COMMENT)->GetValueAsString());
  result->SetDeviceIsVideoOnly( this->GetProperty(mitk::USImageMetadata::PROP_DEV_ISVIDEOONLY));
  result->SetDeviceIsCalibrated(this->GetProperty(mitk::USImageMetadata::PROP_DEV_ISCALIBRATED));
  result->SetProbeName(         this->GetProperty(mitk::USImageMetadata::PROP_PROBE_NAME)->GetValueAsString());
  result->SetProbeFrequency(    this->GetProperty(mitk::USImageMetadata::PROP_PROBE_FREQUENCY)->GetValueAsString());
  result->SetZoom(              this->GetProperty(mitk::USImageMetadata::PROP_ZOOM)->GetValueAsString());

  return result;
}


void mitk::USImage::SetMetadata(mitk::USImageMetadata::Pointer metadata){
  this->SetProperty(mitk::USImageMetadata::PROP_DEV_MANUFACTURER, mitk::StringProperty::New(metadata->GetDeviceManufacturer()));
  this->SetProperty(mitk::USImageMetadata::PROP_DEV_MODEL, mitk::StringProperty::New(metadata->GetDeviceModel()));
  this->SetProperty(mitk::USImageMetadata::PROP_DEV_COMMENT, mitk::StringProperty::New(metadata->GetDeviceComment()));
  this->SetProperty(mitk::USImageMetadata::PROP_DEV_ISVIDEOONLY, mitk::BoolProperty::New(metadata->GetDeviceIsVideoOnly()));
  this->SetProperty(mitk::USImageMetadata::PROP_DEV_ISCALIBRATED, mitk::BoolProperty::New(metadata->GetDeviceIsCalibrated()));
  this->SetProperty(mitk::USImageMetadata::PROP_PROBE_NAME, mitk::StringProperty::New(metadata->GetProbeName()));
  this->SetProperty(mitk::USImageMetadata::PROP_PROBE_FREQUENCY, mitk::StringProperty::New(metadata->GetProbeFrequency()));
  this->SetProperty(mitk::USImageMetadata::PROP_ZOOM, mitk::StringProperty::New(metadata->GetZoom()));
}

