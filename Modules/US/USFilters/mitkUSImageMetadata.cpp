/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUSImageMetadata.h"


const char* mitk::USImageMetadata::PROP_DEV_MANUFACTURER  = "US_Device_Manufacturer";
const char* mitk::USImageMetadata::PROP_DEV_MODEL         = "US_Device_Model";
const char* mitk::USImageMetadata::PROP_DEV_COMMENT       = "US_Device_Comment";
const char* mitk::USImageMetadata::PROP_DEV_ISVIDEOONLY   = "US_Device_VideoOnly";
const char* mitk::USImageMetadata::PROP_PROBE_NAME        = "US_Probe_Name";
const char* mitk::USImageMetadata::PROP_PROBE_FREQUENCY   = "US_Probe_Frequency";
const char* mitk::USImageMetadata::PROP_ZOOM              = "US_Zoom_Factor";


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
}

mitk::USImageMetadata::~USImageMetadata()
{

}

