/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/


#include "mitkVideoSource.h"

mitk::VideoSource::VideoSource()
: m_CaptureWidth(0),
  m_CaptureHeight(0),
  m_CapturingInProcess(false),
  m_CurrentVideoTexture(NULL),
  m_RotationAngle(0.0),
  m_RotationEnabled(false),
  m_FrameCount(0)
{ 
}

mitk::VideoSource::~VideoSource()
{
  if(m_CurrentVideoTexture)
    delete m_CurrentVideoTexture;
}

unsigned char* mitk::VideoSource::GetVideoTexture()
{ // Fetch Frame and return pointer to opengl texture
  return 0;
}

void mitk::VideoSource::StartCapturing()
{   
  m_CapturingInProcess = true;
  m_FrameCount = 0;
  Started.Send();
}

void mitk::VideoSource::StopCapturing()
{
  mitk::VideoSource::m_CapturingInProcess = false;
  m_FrameCount = 0;
  Stopped.Send();
}

bool mitk::VideoSource::IsCapturingEnabled() const
{
  return m_CapturingInProcess;
}

void mitk::VideoSource::FetchFrame()
{
  ++m_FrameCount;
}
