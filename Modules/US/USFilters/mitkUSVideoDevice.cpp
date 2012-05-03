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

#include "mitkUSVideoDevice.h"
#include <string>


mitk::USVideoDevice::USVideoDevice(int videoDeviceNumber, std::string manufacturer, std::string model) : mitk::USDevice(manufacturer, model, true)
{
  m_Source = mitk::USImageVideoSource::New();
  m_Source->SetCameraInput(videoDeviceNumber);
}


mitk::USVideoDevice::~USVideoDevice()
{
  
}

void mitk::USVideoDevice::GenerateData()
{

  mitk::USImage::Pointer result = mitk::USImage::New();
  // Set Metadata
  result->SetMetadata(this->m_Metadata);

  // 1) get Image from Source

  // 2) Process Image as necessary 

  // 3) Set Output

}
