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

  std::cout << "[" << std::setw(7) << std::setprecision(3) << ((double)clock())/CLOCKS_PER_SEC;

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

  std::string msg(l.message);

  int s=msg.size();

  while(s>0 && msg[s-1]=='\n')
      msg = msg.substr(0,--s);

  std::cout << msg << std::endl;

}

void mbilog::BackendCout::FormatFull(const LogMessage &l,int threadID)
{
  printf("%s(%d)",l.filePath,l.lineNumber);

  printf(" func '%s'",l.functionName);

  if(threadID)
  {
    printf(" thread %x",threadID);
  }

  if(strcmp(l.moduleName,"n/a"))
  {
    printf(" module '%s'",l.moduleName);
  }

  if(l.category.size()>0)
  {
    printf(" category '%s'",l.category.c_str());
  }

  printf(" %.3fs\n",((double)clock())/CLOCKS_PER_SEC);

  switch(l.level)
  {
    case mbilog::Info:
      printf("INFO: ");
      break;

    case mbilog::Warn:
      printf("WARN: ");
      break;

    case mbilog::Error:
      printf("ERROR: ");
      break;

    case mbilog::Fatal:
      printf("FATAL: ");
      break;

    case mbilog::Debug:
      printf("DEBUG: ");
      break;
  }

  std::string msg=l.message;

  int s=msg.size();

  if(s>0)
    if(msg[s-1]=='\n')
      msg = msg.substr(0,s-1);

  printf("%s\n",msg.c_str());
}
