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

#include <iostream>
#include <fstream>

static itk::SimpleFastMutexLock logMutex;
static std::ofstream *logFile = 0;

void mitk::LogBackend::ProcessMessage(const mbilog::LogMessage& l )
{
  logMutex.Lock();
  #ifdef _WIN32
    mbilog::BackendCout::FormatSmart( l, (int)GetCurrentThreadId() );
  #else
    mbilog::BackendCout::FormatSmart( l );
  #endif
  
  if(logFile)
  {
    #ifdef _WIN32
      mbilog::BackendCout::FormatFull( *logFile, l, (int)GetCurrentThreadId() );
    #else
      mbilog::BackendCout::FormatFull( *logFile, l );
    #endif
  }
  
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
    SetLogFile(0);
    mbilog::UnregisterBackend( mitkLogBackend );
    delete mitkLogBackend;
    mitkLogBackend=0;
  }
}

mitk::LogBackend *mitk::LogBackend::GetBackend()
{
  return mitkLogBackend;
}


void mitk::LogBackend::SetLogFile(char *file)
{
  logMutex.Lock();
  if(logFile)
  {
    logFile->close();
    delete logFile;
    logFile = 0;
  }
  if(file)
  {
    logFile = new std::ofstream( file,  std::ios_base::out | std::ios_base::trunc  );
  }
  logMutex.Unlock();
}


