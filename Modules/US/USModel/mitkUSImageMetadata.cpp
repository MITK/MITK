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

#include "mitkUSImageMetadata.h"


const char* mitk::USImageMetadata::PROP_DEV_MANUFACTURER  = "US.Device.Manufacturer";
const char* mitk::USImageMetadata::PROP_DEV_MODEL         = "US.Device.Model";
const char* mitk::USImageMetadata::PROP_DEV_COMMENT       = "US.Device.Comment";
const char* mitk::USImageMetadata::PROP_DEV_ISCALIBRATED  = "US.Device.IsCalibrated";
const char* mitk::USImageMetadata::PROP_DEV_ISVIDEOONLY   = "US.Device.VideoOnly";
const char* mitk::USImageMetadata::PROP_PROBE_NAME        = "US.Probe.Name";
const char* mitk::USImageMetadata::PROP_PROBE_FREQUENCY   = "US.Probe.Frequency";
const char* mitk::USImageMetadata::PROP_ZOOM              = "US.Zoom.Factor";
const char* mitk::USImageMetadata::PROP_DEVICE_CLASS      = "org.mitk.modules.us.USVideoDevice";

mitk::USImageMetadata::USImageMetadata() : itk::Object()
{
  // Set Default Values
  this->SetDeviceComment("None");
  this->SetDeviceIsVideoOnly(true);
  this->SetDeviceManufacturer("Unknown Manufacturer");
  this->SetDeviceModel("Unknown Model");
  this->SetProbeFrequency("Unknown Frequency");
  this->SetProbeName("Unknown Probe");
  this->SetZoom("Unknown Zoom Factor");
  m_DeviceIsCalibrated = false;
}

mitk::USImageMetadata::~USImageMetadata()
{

}

