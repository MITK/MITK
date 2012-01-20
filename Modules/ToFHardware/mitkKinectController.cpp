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
  KinectController::KinectController():
    m_Context(NULL),
    m_DepthGenerator(NULL),
    m_ImageGenerator(NULL),
    m_ConnectionCheck(false),
    m_CaptureWidth(640),
    m_CaptureHeight(480)
  {
  }

  KinectController::~KinectController()
  {
  }

  bool KinectController::OpenCameraConnection()
  {
    if (!m_ConnectionCheck)
    {
      MITK_INFO<<"Before init";
      // Initialize the OpenNI status
      m_ConnectionCheck = !ErrorText(m_Context.Init());
      MITK_INFO<<"After init";

      MITK_INFO<<"Before depthgen";
      // Create a depth map generator and set its resolution
      m_ConnectionCheck = !ErrorText(m_DepthGenerator.Create(m_Context));
      MITK_INFO<<"After depthgen";
      XnMapOutputMode DepthMode;
      m_DepthGenerator.GetMapOutputMode(DepthMode);
      DepthMode.nXRes = xn::Resolution((XnResolution)XN_RES_VGA).GetXResolution();
      DepthMode.nYRes = xn::Resolution((XnResolution)XN_RES_VGA).GetYResolution();
      m_ConnectionCheck = !ErrorText(m_DepthGenerator.SetMapOutputMode(DepthMode));
      {
        XnUInt32 NumModes = 10;
        XnMapOutputMode *SupportedModes = new XnMapOutputMode[NumModes];
        m_ConnectionCheck = !ErrorText(m_DepthGenerator.GetSupportedMapOutputModes(SupportedModes, NumModes));
        for ( unsigned int i = 0; i < NumModes; i++ )
        {
          std::cout << "DepthModes #" << i << std::endl;
          std::cout << "	Nx=" << SupportedModes[i].nXRes << std::endl;
          std::cout << "	Ny=" << SupportedModes[i].nYRes << std::endl;
          std::cout << "	FPS=" << SupportedModes[i].nFPS << std::endl;
        }
        delete[] SupportedModes;
      }

      // Create an image generator and set its resolution
      m_ConnectionCheck = !ErrorText(m_ImageGenerator.Create(m_Context));
      XnMapOutputMode ImageMode;
      m_ImageGenerator.GetMapOutputMode(ImageMode);
      ImageMode.nXRes = xn::Resolution((XnResolution)XN_RES_VGA).GetXResolution();
      ImageMode.nYRes = xn::Resolution((XnResolution)XN_RES_VGA).GetYResolution();
      m_ConnectionCheck = !ErrorText(m_ImageGenerator.SetMapOutputMode(ImageMode));
      {
        XnUInt32 NumModes = 10;
        XnMapOutputMode *SupportedModes = new XnMapOutputMode[NumModes];
        m_ConnectionCheck = !ErrorText(m_ImageGenerator.GetSupportedMapOutputModes(SupportedModes, NumModes));
        for ( unsigned int i = 0; i < NumModes; i++ )
        {
          std::cout << "ImageModes #" << i << std::endl;
          std::cout << "	Nx=" << SupportedModes[i].nXRes << std::endl;
          std::cout << "	Ny=" << SupportedModes[i].nYRes << std::endl;
          std::cout << "	FPS=" << SupportedModes[i].nFPS << std::endl;
        }
        delete[] SupportedModes;
      }

      // Camera registration
      if ( m_DepthGenerator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT) )
      {
        m_ConnectionCheck = !ErrorText(m_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint(m_ImageGenerator));
      }
      else
      {
        std::cout << "Alternative view point not supported by the depth generator..." << std::endl;
      }

      // Mirror data
      m_ConnectionCheck = !ErrorText(m_Context.SetGlobalMirror(!m_Context.GetGlobalMirror()));

      // Start data generation
      m_ConnectionCheck = !ErrorText(m_Context.StartGeneratingAll());

      // Update the connected flag
      m_ConnectionCheck = true;
    }
    return m_ConnectionCheck;
  }

  bool KinectController::CloseCameraConnection()
  {
    m_ConnectionCheck = !ErrorText(m_Context.StopGeneratingAll());
    return !m_ConnectionCheck;
  }

  bool KinectController::UpdateCamera()
  {
    bool updateSuccessful = !ErrorText(m_Context.WaitAndUpdateAll());
    xn::DepthMetaData DepthMD;
    m_DepthGenerator.GetMetaData(DepthMD);
    m_CaptureWidth = DepthMD.XRes();
    m_CaptureHeight = DepthMD.YRes();
    return updateSuccessful;
  }

  // TODO flag image
  void KinectController::GetDistances(float* distances)
  {
    xn::DepthMetaData DepthMD;
    m_DepthGenerator.GetMetaData(DepthMD);
    const XnDepthPixel* DepthData = DepthMD.Data();
    for (unsigned int i=0; i<m_CaptureWidth*m_CaptureHeight; i++, ++DepthData, ++distances)
    {
      *distances = *DepthData;
    }
  }

  bool KinectController::ErrorText(unsigned int error)
  {
    if(error != XN_STATUS_OK)
    {
      MITK_ERROR << "Camera Error " << xnGetStatusString(error);
      return false;
    }
    else return true;
  }
}
