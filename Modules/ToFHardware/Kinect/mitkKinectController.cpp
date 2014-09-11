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

#include <XnCppWrapper.h>

namespace mitk
{
class KinectController::KinectControllerPrivate
{
public:
  KinectControllerPrivate();
  ~KinectControllerPrivate();

  bool ErrorText(unsigned int error);
//OpenNI related stuff
  xn::Context m_Context; ///< OpenNI context
  xn::DepthGenerator m_DepthGenerator; ///< Depth generator to access depth image of kinect
  xn::ImageGenerator m_ImageGenerator; ///< Image generator to access RGB image of kinect
  xn::IRGenerator m_IRGenerator; ///< IR generator to access IR image of kinect


  bool m_ConnectionCheck; ///< check if camera is connected or not

  bool m_UseIR; ///< flag indicating whether IR image is used or not

  unsigned int m_CaptureWidth; ///< image width
  unsigned int m_CaptureHeight; ///< image height
};

KinectController::KinectControllerPrivate::KinectControllerPrivate():
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

KinectController::KinectControllerPrivate::~KinectControllerPrivate()
{
}

bool KinectController::KinectControllerPrivate::ErrorText(unsigned int error)
{
  if(error != XN_STATUS_OK)
  {
    MITK_ERROR << "Kinect Camera Error " << xnGetStatusString(error);
    mitkThrow() << "Kinect Camera Error " << xnGetStatusString(error);
    return false;
  }
  else return true;
}

KinectController::KinectController(): d(new KinectControllerPrivate)
{
}

KinectController::~KinectController()
{
    delete d;
  }

  bool KinectController::OpenCameraConnection()
  {
    if (!d->m_ConnectionCheck)
    {
      // Initialize the OpenNI status
      d->m_ConnectionCheck = d->ErrorText(d->m_Context.Init());
      if (!d->m_ConnectionCheck) return false;
      // Create a depth generator and set its resolution
      XnMapOutputMode DepthMode;
      d->m_ConnectionCheck = d->ErrorText(d->m_DepthGenerator.Create(d->m_Context));
      if (!d->m_ConnectionCheck) return false;
      d->m_DepthGenerator.GetMapOutputMode(DepthMode);
      DepthMode.nXRes = xn::Resolution((XnResolution)XN_RES_VGA).GetXResolution();
      DepthMode.nYRes = xn::Resolution((XnResolution)XN_RES_VGA).GetYResolution();
      d->m_ConnectionCheck = d->ErrorText(d->m_DepthGenerator.SetMapOutputMode(DepthMode));
      if (!d->m_ConnectionCheck) return false;

      if (d->m_UseIR)
      {
        // Create the IR generator and set its resolution
        d->m_ConnectionCheck = d->ErrorText(d->m_IRGenerator.Create(d->m_Context));
        if (!d->m_ConnectionCheck) return false;
        XnMapOutputMode IRMode;
        d->m_IRGenerator.GetMapOutputMode(IRMode);
        IRMode.nXRes = XN_VGA_X_RES;
        IRMode.nYRes = XN_VGA_Y_RES;
        IRMode.nFPS = 30;
        d->m_ConnectionCheck = d->ErrorText(d->m_IRGenerator.SetMapOutputMode(IRMode));
        if (!d->m_ConnectionCheck) return false;
      }
      else
      {
        // Create an image generator and set its resolution
        XnMapOutputMode ImageMode;
        d->m_ConnectionCheck = d->ErrorText(d->m_ImageGenerator.Create(d->m_Context));
        if (!d->m_ConnectionCheck) return false;
        d->m_ImageGenerator.GetMapOutputMode(ImageMode);
        ImageMode.nXRes = xn::Resolution((XnResolution)XN_RES_VGA).GetXResolution();
        ImageMode.nYRes = xn::Resolution((XnResolution)XN_RES_VGA).GetYResolution();
        d->m_ConnectionCheck = d->ErrorText(d->m_ImageGenerator.SetMapOutputMode(ImageMode));
        if (!d->m_ConnectionCheck) return false;
      }

      // Camera registration
      if( d->m_DepthGenerator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT) )
      {
        if (!d->m_UseIR)
        {
          d->m_ConnectionCheck = d->ErrorText(d->m_DepthGenerator.GetAlternativeViewPointCap().SetViewPoint(d->m_ImageGenerator));
        }
      }
      else
      {
        MITK_ERROR << "Alternative view point not supported by the depth generator...";
      }
      if(d->m_UseIR)
      {
        if( d->m_IRGenerator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT) )
        {
          d->m_ConnectionCheck = d->ErrorText(d->m_IRGenerator.GetAlternativeViewPointCap().SetViewPoint(d->m_DepthGenerator));
        }
        else
        {
          MITK_ERROR << "Alternative view point not supported by the depth generator...";
        }
      }

      // Start data generation
      d->m_ConnectionCheck = d->ErrorText(d->m_Context.StartGeneratingAll());
      if (!d->m_ConnectionCheck) return false;
    }
    return d->m_ConnectionCheck;
  }

  bool KinectController::CloseCameraConnection()
  {
    d->m_ConnectionCheck = !d->ErrorText(d->m_Context.StopGeneratingAll());
    return !d->m_ConnectionCheck;
  }

  bool KinectController::UpdateCamera()
  {
    bool updateSuccessful = d->ErrorText(d->m_Context.WaitAndUpdateAll());
    xn::DepthMetaData DepthMD;
    d->m_DepthGenerator.GetMetaData(DepthMD);
    d->m_CaptureWidth = DepthMD.XRes();
    d->m_CaptureHeight = DepthMD.YRes();
    return updateSuccessful;
  }

  void KinectController::GetDistances(float* distances)
  {
    xn::DepthMetaData DepthMD;
    d->m_DepthGenerator.GetMetaData(DepthMD);
    const XnDepthPixel* DepthData = DepthMD.Data();

    for (unsigned int i=0; i<d->m_CaptureWidth*d->m_CaptureHeight; i++)
    {
      distances[i] = static_cast<float>(DepthData[i]);
    }
  }

  void KinectController::GetRgb(unsigned char* rgb)
  {
    if (!d->m_UseIR)
    {
      xn::ImageMetaData ImageMD;
      d->m_ImageGenerator.GetMetaData(ImageMD);
      const XnRGB24Pixel* rgbPixelArray = ImageMD.RGB24Data();
      for (int i=0; i<d->m_CaptureWidth*d->m_CaptureHeight; i++)
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
    d->m_DepthGenerator.GetMetaData(DepthMD);
    const XnDepthPixel* DepthData = DepthMD.Data();
    // IR data
    xn::IRMetaData IRData;
    const XnIRPixel* IRPixelData;
    // Image data
    xn::ImageMetaData ImageMD;
    const XnRGB24Pixel* rgbPixelArray;
    if (d->m_UseIR)
    {
      d->m_IRGenerator.GetMetaData(IRData);
      IRPixelData = IRData.Data();
    }
    else
    {
      // get current rgb data
      d->m_ImageGenerator.GetMetaData(ImageMD);
      rgbPixelArray = ImageMD.RGB24Data();
    }

    for (unsigned int i=0; i<d->m_CaptureWidth*d->m_CaptureHeight; i++)
    {
      distances[i] = static_cast<float>(DepthData[i]);
      if (d->m_UseIR)
      {
        amplitudes[i] = static_cast<float>(IRPixelData[i]);
      }
      else
      {
        rgb[i*3] = rgbPixelArray[i].nRed;
        rgb[i*3+1] = rgbPixelArray[i].nGreen;
        rgb[i*3+2] = rgbPixelArray[i].nBlue;
      }
    }
  }

  void KinectController::GetAmplitudes( float* amplitudes )
  {
    if (d->m_UseIR)
    {
      xn::IRMetaData IRData;
      d->m_IRGenerator.GetMetaData(IRData);
      const XnIRPixel* IRPixelData = IRData.Data();

      for (unsigned int i=0; i<d->m_CaptureWidth*d->m_CaptureHeight; i++)
      {
        amplitudes[i] = static_cast<float>(IRPixelData[i]);
      }
    }
  }

  void KinectController::GetIntensities( float* intensities )
  {
  }

  unsigned int KinectController::GetCaptureWidth() const
  {
    return d->m_CaptureWidth;
  }

  unsigned int KinectController::GetCaptureHeight() const
  {
    return d->m_CaptureHeight;
  }

  bool KinectController::GetUseIR() const
  {
    return d->m_UseIR;
  }
  void KinectController::SetUseIR(bool useIR)
  {
    if (d->m_UseIR!=useIR)
    {
      d->m_UseIR = useIR;
      this->Modified();
    }
  }
}
