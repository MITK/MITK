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
#include "mitkKinectController.h"

namespace mitk
{
  KinectController::KinectController()
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
  }

  KinectController::~KinectController()
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
  }

  bool KinectController::OpenCameraConnection()
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
    return true;
  }

  bool KinectController::CloseCameraConnection()
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
    return true;
  }

  bool KinectController::UpdateCamera()
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
    return true;
  }

  // TODO flag image
  void KinectController::GetDistances(float* distances)
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
  }

  void KinectController::GetRgb(unsigned char* rgb)
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
  }

  void KinectController::GetAllData(float* distances, float* amplitudes, unsigned char* rgb)
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
  }

  void KinectController::GetAmplitudes( float* amplitudes )
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
  }

  void KinectController::GetIntensities( float* intensities )
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
  }
}
