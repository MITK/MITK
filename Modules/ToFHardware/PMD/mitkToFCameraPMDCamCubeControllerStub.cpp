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
#include "mitkToFCameraPMDCamCubeController.h"

namespace mitk
{
  ToFCameraPMDCamCubeController::ToFCameraPMDCamCubeController()
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
  }

  ToFCameraPMDCamCubeController::~ToFCameraPMDCamCubeController()
  {
  }

  bool ToFCameraPMDCamCubeController::OpenCameraConnection()
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetDistanceOffset( float offset )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  float mitk::ToFCameraPMDCamCubeController::GetDistanceOffset()
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return 0.0f;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetRegionOfInterest( unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetRegionOfInterest( unsigned int roi[4] )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  unsigned int* mitk::ToFCameraPMDCamCubeController::GetRegionOfInterest()
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return NULL;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetExposureMode( int mode )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetFieldOfView( float fov )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetFPNCalibration( bool on )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetFPPNCalibration( bool on )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetLinearityCalibration( bool on )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }

  bool mitk::ToFCameraPMDCamCubeController::SetLensCalibration( bool on )
  {
    MITK_WARN("ToF") << "Error: PMD Camcube currently not available";
    return true;
  }
}
