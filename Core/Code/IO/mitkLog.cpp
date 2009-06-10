/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Tue, 12 May 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkLog.h"

#include "itkSimpleFastMutexLock.h"

static itk::SimpleFastMutexLock logMutex;

void mitk::LogBackend::ProcessMessage(const mbilog::LogMessage& l )
{
  logMutex.Lock();
  #ifdef _WIN32
    mbilog::BackendCout::FormatSmart( l, (int)GetCurrentThreadId() );
  #else
    mbilog::BackendCout::FormatSmart( l );
  #endif
  logMutex.Unlock();
}

static mitk::LogBackend *mitkLogBackend = 0;

void mitk::LogBackend::Register()
{
  mitkLogBackend = new mitk::LogBackend();
  mbilog::RegisterBackend( mitkLogBackend );
}

void mitk::LogBackend::Unregister()
{
  if(mitkLogBackend)
  {
    mbilog::UnregisterBackend( mitkLogBackend );
    delete mitkLogBackend;
    mitkLogBackend=0;
  }
}

mitk::LogBackend *mitk::LogBackend::GetBackend()
{
  return mitkLogBackend;
}


