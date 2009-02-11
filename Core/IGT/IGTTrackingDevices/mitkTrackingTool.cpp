/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkTrackingTool.h"



mitk::TrackingTool::TrackingTool() 
: itk::Object(), m_ToolName(""), m_ErrorMessage("")
{
  m_MyMutex = itk::FastMutexLock::New();
}

mitk::TrackingTool::~TrackingTool()
{
  m_MyMutex->Unlock();
  m_MyMutex = NULL;
}

const char* mitk::TrackingTool::GetToolName() const
{ 
  m_MyMutex->Lock();
  const char* c = this->m_ToolName.c_str(); 
  m_MyMutex->Unlock();
  return c;
}

const char* mitk::TrackingTool::GetErrorMessage() const
{ 
  m_MyMutex->Lock();
  const char* c = this->m_ErrorMessage.c_str(); 
  m_MyMutex->Unlock();
  return c;
}