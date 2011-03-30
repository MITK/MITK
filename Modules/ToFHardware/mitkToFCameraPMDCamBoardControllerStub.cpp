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
}
