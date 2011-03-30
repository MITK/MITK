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
#include "mitkToFCameraPMDMITKPlayerController.h"

namespace mitk
{
  ToFCameraPMDMITKPlayerController::ToFCameraPMDMITKPlayerController()
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
  }

  ToFCameraPMDMITKPlayerController::~ToFCameraPMDMITKPlayerController()
  {
  }

  bool ToFCameraPMDMITKPlayerController::OpenMITKFile()
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return true;
  }

  bool ToFCameraPMDMITKPlayerController::ReadFile()
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return true;
  }

  bool ToFCameraPMDMITKPlayerController::OpenCameraConnection()
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return true;
  }

  bool ToFCameraPMDMITKPlayerController::CloseCameraConnection()
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return true;
  }

  bool ToFCameraPMDMITKPlayerController::UpdateCamera()
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return true;
  }

  bool ToFCameraPMDMITKPlayerController::GetSourceData(char* sourceDataArray)
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return true;
  }

  int ToFCameraPMDMITKPlayerController::SetIntegrationTime(unsigned int integrationTime)
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return 0;
  }

  int ToFCameraPMDMITKPlayerController::GetIntegrationTime()
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return 0;
  }

  int ToFCameraPMDMITKPlayerController::SetModulationFrequency(unsigned int modulationFrequency)
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return 0;
  }

  int ToFCameraPMDMITKPlayerController::GetModulationFrequency()
  {
    MITK_WARN("ToF") << "Error: MITK TOF Player currently not available. Did you set the directory to your PMD processing plugin?";
    return 0;
  }
}
