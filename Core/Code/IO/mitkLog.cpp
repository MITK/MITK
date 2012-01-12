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
#include "mitkLogMacros.h"

#include "itkSimpleFastMutexLock.h"
#include <itkOutputWindow.h>

#include <iostream>
#include <fstream>

static itk::SimpleFastMutexLock logMutex;
static mitk::LoggingBackend *mitkLogBackend = 0;
static std::ofstream *logFile = 0;
static std::stringstream *outputWindow = 0;
static bool logOutputWindow = false;

void mitk::LoggingBackend::EnableAdditionalConsoleWindow(bool enable)
{
  logOutputWindow = enable;
}


void mitk::LoggingBackend::ProcessMessage(const mbilog::LogMessage& l )
{
  logMutex.Lock();
  #ifdef _WIN32
    FormatSmart( l, (int)GetCurrentThreadId() );
  #else
    FormatSmart( l );
  #endif
  
  if(logFile)
  {
    #ifdef _WIN32
      FormatFull( *logFile, l, (int)GetCurrentThreadId() );
    #else
      FormatFull( *logFile, l );
    #endif
  }
  if(logOutputWindow)
  {
    if(outputWindow == NULL)
    {  outputWindow = new std::stringstream();}
    outputWindow->str("");
    outputWindow->clear();
    #ifdef _WIN32
      FormatFull( *outputWindow, l, (int)GetCurrentThreadId() );
    #else
      FormatFull( *outputWindow, l );
    #endif
    itk::OutputWindow::GetInstance()->DisplayText(outputWindow->str().c_str());
  }
  logMutex.Unlock();
}

void mitk::LoggingBackend::Register()
{
  if(mitkLogBackend)
    return;
  mitkLogBackend = new mitk::LoggingBackend();
  mbilog::RegisterBackend( mitkLogBackend );
}

void mitk::LoggingBackend::Unregister()
{
  if(mitkLogBackend)
  {
    SetLogFile(0);
    mbilog::UnregisterBackend( mitkLogBackend );
    delete mitkLogBackend;
    mitkLogBackend=0;
  }
}

void mitk::LoggingBackend::SetLogFile(const char *file)
{
  logMutex.Lock();
  if(logFile)
  {
    MITK_INFO << "closing logfile";
    logFile->close();
    delete logFile;
    logFile = 0;
  }
  if(file)
  {
      logFile = new std::ofstream( file,  std::ios_base::out | std::ios_base::app );
    /*
    if(*logFile)
    {
      std::cout << "opening logfile '" << file << "' for writing failed";
      MITK_INFO << "logging to '" << file << "'";
    }
    else
    {
      std::cerr << "opening logfile '" << file << "' for writing failed";
      MITK_ERROR << "opening logfile '" << file << "' for writing failed";
      delete logFile;
      logFile = 0;
    }
    */
  }
  logMutex.Unlock();
}

void mitk::LoggingBackend::CatchLogFileCommandLineParameter(int &argc,char **argv)
{
  int r;

  for(r=1;r<argc;r++)
  {
    if(std::string(argv[r])=="--logfile")
    {
      if(r+1>=argc)
      {
        --argc;
        MITK_ERROR << "--logfile parameter found, but no file given";
        return;
      }

      mitk::LoggingBackend::SetLogFile(argv[r+1]);
     
      for(r+=2;r<argc;r++)
        argv[r-2]=argv[r];
        
      argc-=2;     
      return;
    }
  }
}
