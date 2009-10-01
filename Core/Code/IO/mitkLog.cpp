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
static mitk::LogBackend *mitkLogBackend = 0;
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

void mitk::LogBackend::Register()
{
  if(mitkLogBackend)
    return;
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

void mitk::LogBackend::SetLogFile(const char *file)
{
  logMutex.Lock();
  if(logFile)
  {
    LOG_INFO << "closing logfile";
    logFile->close();
    delete logFile;
    logFile = 0;
  }
  if(file)
  {
    logFile = new std::ofstream( file,  std::ios_base::out | std::ios_base::trunc  );
    if(logFile->fail())
    {
      LOG_ERROR << "opening logfile '" << file << "' for writing failed";
      delete logFile;
      logFile = 0;
    }
    else
      LOG_INFO << "logging to '" << file << "'";
  }
  logMutex.Unlock();
}

void mitk::LogBackend::CatchLogFileCommandLineParameter(int &argc,char **argv)
{
  int r;
  
  for(r=1;r<argc;r++)
  {
    if(std::string(argv[r])=="--logfile")
    {
      if(r+1>=argc)
      {
        --argc;
        LOG_ERROR << "--logfile parameter found, but no file given";
        return;
      }

      mitk::LogBackend::SetLogFile(argv[r+1]);
     
      for(r+=2;r<argc;r++)
        argv[r-2]=argv[r];
        
      argc-=2;     
      return;
    }
  }
}
