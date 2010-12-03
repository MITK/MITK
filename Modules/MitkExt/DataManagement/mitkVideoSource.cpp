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
:
  m_CurrentVideoTexture(NULL),
  m_CaptureWidth(0),
  m_CaptureHeight(0),
  m_CapturingInProcess(false),
  m_FrameCount(0),
  m_CapturePaused(false)
{
}

mitk::VideoSource::~VideoSource()
{
  if(m_CurrentVideoTexture)
    delete m_CurrentVideoTexture;
}

void mitk::VideoSource::StartCapturing()
{
  m_CapturingInProcess = true;
  m_FrameCount = 0;
  this->Modified();
}

void mitk::VideoSource::StopCapturing()
{
  m_CapturingInProcess = false;
  m_FrameCount = 0;
  this->Modified();
}

bool mitk::VideoSource::IsCapturingEnabled() const
{
  return m_CapturingInProcess;
}

void mitk::VideoSource::FetchFrame()
{
  ++m_FrameCount;
  this->Modified();
}

int mitk::VideoSource::GetImageWidth()
{
  return m_CaptureWidth;
}

int mitk::VideoSource::GetImageHeight()
{
  return m_CaptureHeight;
}

unsigned long mitk::VideoSource::GetFrameCount() const
{
  return m_FrameCount;
}

bool mitk::VideoSource::GetCapturePaused() const
{
  return m_CapturePaused;
}

void mitk::VideoSource::PauseCapturing()
{
  m_CapturePaused = !m_CapturePaused;
}
