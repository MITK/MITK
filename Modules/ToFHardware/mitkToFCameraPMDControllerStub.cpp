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
#include "mitkToFCameraPMDController.h"

namespace mitk
{
  ToFCameraPMDController::ToFCameraPMDController()
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
  }

  ToFCameraPMDController::~ToFCameraPMDController()
  {
  }

  bool ToFCameraPMDController::CloseCameraConnection()
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  } 

  bool ToFCameraPMDController::ErrorText(int error)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  bool ToFCameraPMDController::UpdateCamera()
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  bool ToFCameraPMDController::GetAmplitudes(float* amplitudeArray)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  bool ToFCameraPMDController::GetAmplitudes(char* sourceData, float* amplitudeArray)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  bool ToFCameraPMDController::GetIntensities(float* intensityArray)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  bool ToFCameraPMDController::GetIntensities(char* sourceData, float* intensityArray)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  bool ToFCameraPMDController::GetDistances(float* distanceArray)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  bool ToFCameraPMDController::GetSourceData(char* sourceDataArray)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  bool ToFCameraPMDController::GetDistances(char* sourceData, float* distanceArray)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return false;
  }

  int ToFCameraPMDController::SetIntegrationTime(unsigned int integrationTime)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return 0;
  }

  int ToFCameraPMDController::GetIntegrationTime()
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return 0;
  }

  int ToFCameraPMDController::SetModulationFrequency(unsigned int modulationFrequency)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return 0;
  }

  int ToFCameraPMDController::GetModulationFrequency()
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
    return 0;
  }

  void ToFCameraPMDController::SetInputFileName(std::string inputFileName)
  {
    MITK_WARN("ToF") << "Error: PMD devices currently not available";
  }
}
