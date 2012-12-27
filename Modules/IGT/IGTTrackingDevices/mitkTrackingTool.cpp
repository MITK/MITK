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
