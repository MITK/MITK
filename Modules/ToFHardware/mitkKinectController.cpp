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
    m_IRGenerator(NULL),
    m_ConnectionCheck(false),
    m_UseIR(false),
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
      // Initialize the OpenNI status
      m_ConnectionCheck = !ErrorText(m_Context.Init());
      // Create a depth generator and set its resolution
      XnMapOutputMode DepthMode;
      m_ConnectionCheck = !ErrorText(m_DepthGenerator.Create(m_Context));
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
          std::cout << "Nx=" << SupportedModes[i].nXRes << std::endl;
          std::cout << "Ny=" << SupportedModes[i].nYRes << std::endl;
          std::cout << "FPS=" << SupportedModes[i].nFPS << std::endl;
        }
        delete[] SupportedModes;
      }

      if (m_UseIR)
      {
        // Create the IR generator and set its resolution
        m_ConnectionCheck = !ErrorText(m_IRGenerator.Create(m_Context));
        XnMapOutputMode IRMode;
        m_IRGenerator.GetMapOutputMode(IRMode);
        IRMode.nXRes = XN_VGA_X_RES;
        IRMode.nYRes = XN_VGA_Y_RES;
        IRMode.nFPS = 30;
        m_ConnectionCheck = !ErrorText(m_IRGenerator.SetMapOutputMode(IRMode));

        XnUInt32 NumModes = 10;
        XnMapOutputMode *SupportedModes = new XnMapOutputMode[NumModes];
        m_ConnectionCheck = !ErrorText(m_IRGenerator.GetSupportedMapOutputModes(SupportedModes, NumModes));
        for ( unsigned int i = 0; i < NumModes; i++ )
        {
          std::cout << "Mode #" << i << std::endl;
          std::cout << "Nx=" << SupportedModes[i].nXRes << std::endl;
          std::cout << "Ny=" << SupportedModes[i].nYRes << std::endl;
          std::cout << "FPS=" << SupportedModes[i].nFPS << std::endl;
        }
        delete[] SupportedModes;
      }
      else
      {
        // Create an image generator and set its resolution
        XnMapOutputMode ImageMode;
        m_ConnectionCheck = !ErrorText(m_ImageGenerator.Create(m_Context));
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
            std::cout << "Nx=" << SupportedModes[i].nXRes << std::endl;
            std::cout << "Ny=" << SupportedModes[i].nYRes << std::endl;
            std::cout << "FPS=" << SupportedModes[i].nFPS << std::endl;
          }
          delete[] SupportedModes;
        }
      }

      // Camera registration
      if ( m_DepthGenerator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT) )
      {
        if (m_UseIR)
        {
          m_ConnectionCheck = !ErrorText(m_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint(m_IRGenerator));
        }
        else
        {
          m_ConnectionCheck = !ErrorText(m_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint(m_ImageGenerator));
        }
      }
      else
      {
        std::cout << "Alternative view point not supported by the depth generator..." << std::endl;
      }
      if (m_UseIR)
      {
        if ( m_IRGenerator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT) )
        {
          m_ConnectionCheck = !ErrorText(m_IRGenerator.GetAlternativeViewPointCap().SetViewPoint(m_DepthGenerator));
        }
        else
        {
          std::cout << "Alternative view point not supported by the depth generator..." << std::endl;
        }
      }

      // Mirror data
      m_ConnectionCheck = ErrorText(m_Context.SetGlobalMirror(!m_Context.GetGlobalMirror()));

      // Start data generation
      m_ConnectionCheck = ErrorText(m_Context.StartGeneratingAll());

//      // Update the connected flag
//      m_ConnectionCheck = true;
    }
    MITK_INFO<<"Controller connect?"<<m_ConnectionCheck;
    return m_ConnectionCheck;
  }

  bool KinectController::CloseCameraConnection()
  {
    m_ConnectionCheck = !ErrorText(m_Context.StopGeneratingAll());
    return !m_ConnectionCheck;
  }

  bool KinectController::UpdateCamera()
  {
    bool updateSuccessful = ErrorText(m_Context.WaitAndUpdateAll());
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

    for (unsigned int i=0; i<m_CaptureWidth*m_CaptureHeight; i++)
    {
      distances[i] = DepthData[i];
    }
  }

  void KinectController::GetRgb(unsigned char* rgb)
  {
    if (!m_UseIR)
    {
      xn::ImageMetaData ImageMD;
      m_ImageGenerator.GetMetaData(ImageMD);
      const XnRGB24Pixel* rgbPixelArray = ImageMD.RGB24Data();
      for (int i=0; i<m_CaptureWidth*m_CaptureHeight; i++)
      {
        rgb[i*3] = rgbPixelArray[i].nRed;
        rgb[i*3+1] = rgbPixelArray[i].nGreen;
        rgb[i*3+2] = rgbPixelArray[i].nBlue;
      }
    }
  }

  void KinectController::GetAllData(float* distances, float* amplitudes, unsigned char* rgb)
  {
    // get current distance data
    xn::DepthMetaData DepthMD;
    m_DepthGenerator.GetMetaData(DepthMD);
    const XnDepthPixel* DepthData = DepthMD.Data();
    // IR data
    xn::IRMetaData IRData;
    const XnIRPixel* IRPixelData;
    // Image data
    xn::ImageMetaData ImageMD;
    const XnRGB24Pixel* rgbPixelArray;
    if (m_UseIR)
    {
      m_IRGenerator.GetMetaData(IRData);
      IRPixelData = IRData.Data();
    }
    else
    {
      // get current rgb data
      m_ImageGenerator.GetMetaData(ImageMD);
      rgbPixelArray = ImageMD.RGB24Data();
    }

    for (unsigned int i=0; i<m_CaptureWidth*m_CaptureHeight; i++)
    {
      distances[i] = DepthData[i];
      if (m_UseIR)
      {
        amplitudes[i] = IRPixelData[i];
      }
      else
      {
        rgb[i*3] = rgbPixelArray[i].nRed;
        rgb[i*3+1] = rgbPixelArray[i].nGreen;
        rgb[i*3+2] = rgbPixelArray[i].nBlue;
      }
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

  void KinectController::GetAmplitudes( float* amplitudes )
  {
    if (m_UseIR)
    {
      xn::IRMetaData IRData;
      m_IRGenerator.GetMetaData(IRData);
      const XnIRPixel* IRPixelData = IRData.Data();

      for (unsigned int i=0; i<m_CaptureWidth*m_CaptureHeight; i++)
      {
        amplitudes[i] = IRPixelData[i];
      }
    }
  }

  void KinectController::GetIntensities( float* intensities )
  {

  }
}
