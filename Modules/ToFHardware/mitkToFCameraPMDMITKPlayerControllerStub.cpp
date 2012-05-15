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
