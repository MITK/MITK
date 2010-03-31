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

#include "mitkTrackingTool.h"
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


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
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  return this->m_ToolName.c_str();
}


const char* mitk::TrackingTool::GetErrorMessage() const
{ 
 MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
 return this->m_ErrorMessage.c_str();
}
