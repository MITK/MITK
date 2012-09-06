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

  unsigned int KinectController::GetCaptureWidth() const
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
    return 640;
  }

  unsigned int KinectController::GetCaptureHeight() const
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
    return 480;
  }

  bool KinectController::GetUseIR() const
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
    return true;
  }

  void KinectController::SetUseIR(bool useIR)
  {
    MITK_WARN("ToF") << "Error: Kinect camera currently not available";
  }
}
