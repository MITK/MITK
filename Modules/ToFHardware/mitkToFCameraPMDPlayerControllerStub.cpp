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
#include "mitkToFCameraPMDPlayerController.h"

namespace mitk
{
  ToFCameraPMDPlayerController::ToFCameraPMDPlayerController()
  {
    MITK_WARN("ToF") << "Error: PMD TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
  }

  ToFCameraPMDPlayerController::~ToFCameraPMDPlayerController()
  {
  }

  bool ToFCameraPMDPlayerController::OpenCameraConnection()
  {
    MITK_WARN("ToF") << "Error: PMD TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return false;
  }

  int ToFCameraPMDPlayerController::SetIntegrationTime(unsigned int integrationTime)
  {
    MITK_WARN("ToF") << "Error: PMD TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return 0;
  }

  int ToFCameraPMDPlayerController::SetModulationFrequency(unsigned int modulationFrequency)
  {
    MITK_WARN("ToF") << "Error: PMD TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return 0;
  }

}
