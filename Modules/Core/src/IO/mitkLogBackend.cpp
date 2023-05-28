/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkExceptionMacro.h>
#include <mitkLogBackend.h>

#include <itkOutputWindow.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <mutex>

static std::mutex logMutex;
static mitk::LogBackend *mitkLogBackend = nullptr;
static std::ofstream *logFile = nullptr;
static std::string logFileName = "";
static std::stringstream *outputWindow = nullptr;
static bool logOutputWindow = false;

void mitk::LogBackend::EnableAdditionalConsoleWindow(bool enable)
{
  logOutputWindow = enable;
}

void mitk::LogBackend::ProcessMessage(const LogMessage& message)
{
  logMutex.lock();
#ifdef _WIN32
  FormatSmart(message, (int)GetCurrentThreadId());
#else
  FormatSmart(message);
#endif

  if (logFile)
  {
#ifdef _WIN32
    FormatFull(*logFile, message, (int)GetCurrentThreadId());
#else
    FormatFull(*logFile, message);
#endif
  }
  if (logOutputWindow)
  {
    if (outputWindow == nullptr)
    {
      outputWindow = new std::stringstream();
    }
    outputWindow->str("");
    outputWindow->clear();
#ifdef _WIN32
    FormatFull(*outputWindow, message, (int)GetCurrentThreadId());
#else
    FormatFull(*outputWindow, message);
#endif
    itk::OutputWindow::GetInstance()->DisplayText(outputWindow->str().c_str());
  }
  logMutex.unlock();
}

void mitk::LogBackend::Register()
{
  if (mitkLogBackend)
    return;
  mitkLogBackend = new LogBackend;
  RegisterBackend(mitkLogBackend);
}

void mitk::LogBackend::Unregister()
{
  if (mitkLogBackend)
  {
    SetLogFile("");
    UnregisterBackend(mitkLogBackend);
    delete mitkLogBackend;
    mitkLogBackend = nullptr;
  }
}

void mitk::LogBackend::SetLogFile(const std::string& file)
{
  // closing old logfile
  {
    bool closed = false;
    std::string closedFileName;

    logMutex.lock();
    if (logFile)
    {
      closed = true;
      closedFileName = logFileName;
      logFile->close();
      delete logFile;
      logFile = nullptr;
      logFileName = "";
    }
    logMutex.unlock();
    if (closed)
    {
      MITK_INFO << "closing logfile (" << closedFileName << ")";
    }
  }

  // opening new logfile
  if (!file.empty())
  {
    logMutex.lock();

    logFileName = file;
    logFile = new std::ofstream();

    logFile->open(file, std::ios_base::out | std::ios_base::app);

    if (logFile->good())
    {
      logMutex.unlock();
      MITK_INFO << "Logfile: " << logFileName;
    }
    else
    {
      delete logFile;
      logFile = nullptr;
      logMutex.unlock();
      MITK_WARN << "opening logfile '" << file << "' for writing failed";
    }

    // mutex is now unlocked
  }
}

std::string mitk::LogBackend::GetLogFile()
{
  return logFileName;
}

void mitk::LogBackend::CatchLogFileCommandLineParameter(int &argc, char **argv)
{
  int r;

  for (r = 1; r < argc; r++)
  {
    if (std::string(argv[r]) == "--logfile")
    {
      if (r + 1 >= argc)
      {
        --argc;
        MITK_ERROR << "--logfile parameter found, but no file given";
        return;
      }

      SetLogFile(argv[r + 1]);

      for (r += 2; r < argc; r++)
        argv[r - 2] = argv[r];

      argc -= 2;
      return;
    }
  }
}

void mitk::LogBackend::RotateLogFiles(const std::string &prefixPath)
{
  static const int numLogFiles = 10;
  std::string newEmptyLogFileName;

  // first: rotate the old log files to get a new, free logfile name
  newEmptyLogFileName = IncrementLogFileNames(prefixPath, numLogFiles);

  // now: use the new empty logfile name as name for this run
  SetLogFile(newEmptyLogFileName.c_str());
}

std::string mitk::LogBackend::IncrementLogFileNames(const std::string &prefixPath, int numLogFiles)
{
  // delete last one
  {
    std::stringstream s;
    s << prefixPath.c_str() << "-" << numLogFiles - 1 << ".log";
    // check if the file exists
    if (CheckIfFileExists(s.str())) // if yes: delete it
    {
      int retVal = ::remove(s.str().c_str());
      if (retVal != 0)
      {
        mitkThrow()
          << "Problem while deleting the oldest log file. Maybe the access to this files is blocked. Aborting!";
      }
    }
  }

  // rename the others
  for (int r = numLogFiles - 1; r >= 1; r--)
  {
    std::stringstream dst;
    dst << prefixPath.c_str() << "-" << r << ".log";

    std::stringstream src;
    src << prefixPath.c_str() << "-" << r - 1 << ".log";

    // check if the source exists
    if (CheckIfFileExists(src.str())) // if yes: rename it
    {
      int retVal = ::rename(src.str().c_str(), dst.str().c_str());
      if (retVal != 0)
      {
        mitkThrow() << "Problem while renaming the log files. Maybe the access to this files is blocked. Aborting!";
      }
    }
  }

  // create new empty name and return it
  {
    std::stringstream s;
    s << prefixPath.c_str() << "-0.log";
    return s.str();
  }
}

bool mitk::LogBackend::CheckIfFileExists(const std::string &filename)
{
  bool returnValue = false;
  std::ifstream File(filename.c_str());
  if (File.good())
  {
    returnValue = true;
  }
  else
  {
    returnValue = false;
  }
  File.close();
  return returnValue;
}

mitk::LogBackendBase::OutputType mitk::LogBackend::GetOutputType() const
{
  return OutputType::Console;
}
