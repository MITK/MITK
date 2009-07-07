/*=========================================================================

Program:   mbilog - logging for mitk / openCherry

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <list>
#include <ctime>
#include <string>
#include <iomanip>

#include "mbilog.h"

static std::list<mbilog::AbstractBackend*> backends;

namespace mbilog {
static const std::string NA_STRING = "n/a";
}

void mbilog::RegisterBackend(mbilog::AbstractBackend* backend)
{
  backends.push_back(backend);
}

void mbilog::UnregisterBackend(mbilog::AbstractBackend* backend)
{
  backends.remove(backend);
}

static bool warningNoBackend = false;

void mbilog::DistributeToBackends(const mbilog::LogMessage &l)
{
  if(backends.empty())
  {
    if(!warningNoBackend)
    {
      warningNoBackend = true;
      std::cout << "mbilog ... receiving log messages, but still no backend registered, sending to std::cout\n";
    }
    mbilog::BackendCout::FormatFull(l);
    return;
  }

  std::list<mbilog::AbstractBackend*>::iterator i;

  for(i = backends.begin(); i != backends.end(); i++)
    (*i)->ProcessMessage(l);
}

mbilog::BackendCout::BackendCout()
{
  useFullOutput=false;
}

mbilog::BackendCout::~BackendCout()
{
}

void mbilog::BackendCout::SetFull(bool full)
{
  useFullOutput = full;
}

void mbilog::BackendCout::ProcessMessage(const mbilog::LogMessage& l)
{
  if(useFullOutput)
    FormatFull(l);
  else
    FormatSmart(l);
}

void mbilog::BackendCout::FormatSmart(const LogMessage &l,int threadID)
{
  switch(l.level)
  {
    case mbilog::Info:
      std::cout << "INFO";
      break;

    case mbilog::Warn:
      std::cout << "WARN";
      break;

    case mbilog::Error:
      std::cout << "ERROR";
      break;

    case mbilog::Fatal:
      std::cout << "FATAL";
      break;

    case mbilog::Debug:
      std::cout << "DEBUG";
      break;
  }

  std::cout << "[" << std::setw(7) << std::setprecision(3) << ((double)std::clock())/CLOCKS_PER_SEC;

  if(threadID)
  {
    std::cout << ":" << std::hex << threadID;
  }

  if(!l.category.empty())
  {
    std::cout << ":" << l.category;
  }
  else
  {
    if(NA_STRING != l.moduleName)
    {
      std::cout << ":" << std::string(l.moduleName);
    }
  }

  std::cout << "] ";

  std::size_t i = l.message.find_last_not_of(" \t\f\v\n\r");
  std::cout << ((i =! std::string::npos) ? l.message.substr(0, i+1) : "") << std::endl;

}

void mbilog::BackendCout::FormatFull(const LogMessage &l,int threadID)
{
  std::cout << std::string(l.filePath) << "(" << l.lineNumber << ")";

  std::cout << " func '" << std::string(l.functionName) << "'";

  if(threadID)
  {
    std::cout << " thread " << std::hex << threadID;
  }

  if(NA_STRING != l.moduleName)
  {
    std::cout << " module '" << std::string(l.moduleName) << "'";
  }

  if(!l.category.empty())
  {
    std::cout << " category '" << l.category << "'";
  }

  std::cout << std::setprecision(3) << ((double)std::clock())/CLOCKS_PER_SEC << "s\n";

  switch(l.level)
  {
    case mbilog::Info:
      std::cout << "INFO: ";
      break;

    case mbilog::Warn:
      std::cout << "WARN: ";
      break;

    case mbilog::Error:
      std::cout << "ERROR: ";
      break;

    case mbilog::Fatal:
      std::cout << "FATAL: ";
      break;

    case mbilog::Debug:
      std::cout << "DEBUG: ";
      break;
  }

  std::size_t i = l.message.find_last_not_of(" \t\f\v\n\r");
  std::cout << ((i != std::string::npos) ? l.message.substr(0, i+1) : "") << std::endl;
}
