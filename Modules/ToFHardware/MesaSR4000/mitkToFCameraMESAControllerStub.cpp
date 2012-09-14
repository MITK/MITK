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
#include "mitkToFCameraMESAController.h"

namespace mitk
{
  ToFCameraMESAController::ToFCameraMESAController()
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
  }

  ToFCameraMESAController::~ToFCameraMESAController()
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
  }

  bool ToFCameraMESAController::CloseCameraConnection()
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
    return true;
  }

  bool ToFCameraMESAController::ErrorText(int error)
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
    return true;
  }

  bool ToFCameraMESAController::UpdateCamera()
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
    return true;
  }

  bool ToFCameraMESAController::GetAmplitudes(float* amplitudeArray)
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
    return true;
  }

  bool ToFCameraMESAController::GetIntensities(float* intensityArray)
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
    return true;
  }

  bool ToFCameraMESAController::GetDistances(float* distanceArray)
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
    return true;
  }

  void ToFCameraMESAController::SetInputFileName(std::string inputFileName)
  {
    MITK_WARN("ToF") << "Error: MESA camera currently not available";
  }
}
