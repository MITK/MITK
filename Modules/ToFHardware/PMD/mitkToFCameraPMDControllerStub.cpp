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
#include "mitkToFCameraPMDController.h"

namespace mitk
{
  ToFCameraPMDController::ToFCameraPMDController():m_PMDRes(0), m_PixelNumber(40000), m_NumberOfBytes(0),
    m_CaptureWidth(200), m_CaptureHeight(200), m_SourceDataSize(0), m_SourceDataStructSize(0), m_ConnectionCheck(false),
    m_InputFileName("")
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
  }

  ToFCameraPMDController::~ToFCameraPMDController()
  {
  }

  bool ToFCameraPMDController::CloseCameraConnection()
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::ErrorText(int error)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::UpdateCamera()
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::GetAmplitudes(float* amplitudeArray)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::GetAmplitudes(char* sourceData, float* amplitudeArray)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::GetIntensities(float* intensityArray)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::GetIntensities(char* sourceData, float* intensityArray)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::GetDistances(float* distanceArray)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::GetSourceData(char* sourceDataArray)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::GetShortSourceData(short* sourceData)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  bool ToFCameraPMDController::GetDistances(char* sourceData, float* distanceArray)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return true;
  }

  int ToFCameraPMDController::SetIntegrationTime(unsigned int integrationTime)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return 0;
  }

  int ToFCameraPMDController::GetIntegrationTime()
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return 0;
  }

  int ToFCameraPMDController::SetModulationFrequency(unsigned int modulationFrequency)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return 0;
  }

  int ToFCameraPMDController::GetModulationFrequency()
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
    return 0;
  }

  void ToFCameraPMDController::SetInputFileName(std::string inputFileName)
  {
    MITK_WARN("ToF") << "PMD devices currently not available";
  }
}
