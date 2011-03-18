/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
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
