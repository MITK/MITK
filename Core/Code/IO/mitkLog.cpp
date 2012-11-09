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

#include "mitkLog.h"
#include "mitkLogMacros.h"

#include "itkSimpleFastMutexLock.h"
#include <itkOutputWindow.h>

#include <iostream>
#include <fstream>

static itk::SimpleFastMutexLock logMutex;
static mitk::LoggingBackend *mitkLogBackend = 0;
static std::ofstream *logFile = 0;
static std::string logFileName = "";
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
  // closing old logfile
  {
    bool closed = false;
    std::string closedFileName;

    logMutex.Lock();
    if(logFile)
    {
      closed = true;
      closedFileName = logFileName;
      logFile->close();
      delete logFile;
      logFile = 0;
      logFileName = "";
    }
    logMutex.Unlock();
    if(closed)
    {
      MITK_INFO << "closing logfile (" << closedFileName << ")" ;
    }
  }

  // opening new logfile
  if(file)
  {
    logMutex.Lock();

    logFileName = file;
    logFile = new std::ofstream( );

    logFile->open( file,  std::ios_base::out | std::ios_base::app );

    if(logFile->good())
    {
      logMutex.Unlock();
      MITK_INFO << "Logfile: " << logFileName ;
    }
    else
    {
      delete logFile;
      logFile = 0;
      logMutex.Unlock();
      MITK_WARN << "opening logfile '" << file << "' for writing failed";
    }

    // mutex is now unlocked
  }
}

std::string mitk::LoggingBackend::GetLogFile()
{
  return logFileName;
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
