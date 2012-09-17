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

#include "mitkToFCameraPMDCamBoardController.h"

namespace mitk
{
  ToFCameraPMDCamBoardController::ToFCameraPMDCamBoardController()
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
  }

  ToFCameraPMDCamBoardController::~ToFCameraPMDCamBoardController()
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
  }

  bool ToFCameraPMDCamBoardController::OpenCameraConnection()
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetDistanceOffset( float offset )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  float mitk::ToFCameraPMDCamBoardController::GetDistanceOffset()
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return 0.0;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetRegionOfInterest( unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetRegionOfInterest( unsigned int roi[4] )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  unsigned int* mitk::ToFCameraPMDCamBoardController::GetRegionOfInterest()
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return NULL;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetExposureMode( int mode )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetFieldOfView( float fov )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetFPNCalibration( bool on )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetFPPNCalibration( bool on )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetLinearityCalibration( bool on )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamBoardController::SetLensCalibration( bool on )
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool ToFCameraPMDCamBoardController::GetAmplitudes(float* amplitudeArray)
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool ToFCameraPMDCamBoardController::GetAmplitudes(char* sourceData, float* amplitudeArray)
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool ToFCameraPMDCamBoardController::GetIntensities(float* intensityArray)
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool ToFCameraPMDCamBoardController::GetIntensities(char* sourceData, float* intensityArray)
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool ToFCameraPMDCamBoardController::GetDistances(float* distanceArray)
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  bool ToFCameraPMDCamBoardController::GetDistances(char* sourceData, float* distanceArray)
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
    return true;
  }

  void ToFCameraPMDCamBoardController::TransformCameraOutput(float* in, float* out, bool isDist)
  {
    MITK_WARN("ToF") << "Error: PMD CamBoard currently not available";
  }

}
