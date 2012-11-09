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

#include "mitkVideoInputSource.h"

#include <iostream>
#include <algorithm>
#include <videoInput.h>

mitk::VideoInputSource::VideoInputSource()
: m_ShowSettingsWindow(false)
{


  m_CaptureWidth  = 1024;
  m_CaptureHeight = 720;
  m_CapturingInProcess = false;

  m_DeviceNumber = -1;
  m_CaptureSize = 0;

  m_CurrentVideoTexture   = NULL;
  m_CurrentImage          = NULL;
  m_VideoInput = new videoInput;


  m_UndistortImage        = false;
}

mitk::VideoInputSource::~VideoInputSource()
{
  m_VideoInput->stopDevice(m_DeviceNumber);
  m_CapturingInProcess = false;
  delete m_VideoInput;
}

void mitk::VideoInputSource::FetchFrame()
{ // main procedure for updating video data

//  if(m_CurrentVideoTexture == NULL)
//    m_CurrentVideoTexture = new unsigned char[m_CaptureSize];

  if(m_CapturingInProcess && !m_CapturePaused)
  {
    if(m_CurrentImage == NULL)
      m_CurrentImage = cvCreateImage(cvSize(m_CaptureWidth,m_CaptureHeight),IPL_DEPTH_8U,3);

    if(m_VideoInput->isFrameNew(m_DeviceNumber))
    {
      //m_VideoInput->getPixels(m_DeviceNumber, m_CurrentVideoTexture, true, false);
      m_VideoInput->getPixels(m_DeviceNumber, reinterpret_cast<unsigned char*>(m_CurrentImage->imageData), false, true);
      // only undistort if not paused
      if(m_UndistortImage && m_UndistortCameraImage.IsNotNull())
        m_UndistortCameraImage->UndistortImageFast(m_CurrentImage, 0);
    }
  }
}

void mitk::VideoInputSource::StartCapturing()
{
  //Prints out a list of available devices and returns num of devices found
  int numDevices = m_VideoInput->listDevices();

  try
  {
    m_VideoInput->setupDevice(m_DeviceNumber, m_CaptureWidth, m_CaptureHeight, VI_COMPOSITE);
  }
  catch(...)
  {
    MITK_WARN << "error setting up device";
  }

  //to get a settings dialog for the device
  if(m_ShowSettingsWindow)
    m_VideoInput->showSettingsWindow(m_DeviceNumber);

  //As requested width and height can not always be accomodated
  //make sure to check the size once the device is setup

  m_CaptureWidth  = m_VideoInput->getWidth(m_DeviceNumber);
  m_CaptureHeight   = m_VideoInput->getHeight(m_DeviceNumber);
  m_CaptureSize  = m_VideoInput->getSize(m_DeviceNumber);

  MITK_INFO << "starting capturing with VideoInputLib. Size: " << m_CaptureWidth << " x " << m_CaptureHeight;
  m_CapturingInProcess = true;

}

void mitk::VideoInputSource::StopCapturing()
{
  MITK_INFO << "stopping cpaturing process";
  m_VideoInput->stopDevice(m_DeviceNumber);
  m_CapturingInProcess = false;

}

void mitk::VideoInputSource::SetVideoCameraInput(int cameraindex, bool useCVCAMLib)
{
  m_DeviceNumber = cameraindex;
}
void mitk::VideoInputSource::SetVideoFileInput(const char * filename, bool repeatVideo, bool useCVCAMLib)
{
}

