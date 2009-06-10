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

#include "mbilog.h"

static std::list<mbilog::AbstractBackend*> backends;
  
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
      printf("mbilog ... receiving log messages, but still no backend registered, sending to std::cout\n");
    }  
    mbilog::backendCout::formatFull(l);
    return;
  }

  std::list<mbilog::AbstractBackend*>::iterator i;
   
  for(i = backends.begin(); i != backends.end(); i++)
    (*i)->ProcessMessage(l);
}

mbilog::backendCout::backendCout()
{
  useFullOutput=false;
}

mbilog::backendCout::~backendCout()
{
}

void mbilog::backendCout::setFull(bool full)
{
  useFullOutput = full;
}

void mbilog::backendCout::ProcessMessage(const mbilog::LogMessage& l)
{
  if(useFullOutput)
    formatFull(l);
  else
    formatSmart(l);
}
  
void mbilog::backendCout::formatSmart(const LogMessage &l,int threadID)
{
  switch(l.level)
  {
    case mbilog::Info:
      printf("INFO");
      break;
      
    case mbilog::Warn:
      printf("WARN");
      break;
      
    case mbilog::Error:
      printf("ERROR");
      break;
      
    case mbilog::Fatal:
      printf("FATAL");
      break;
      
    case mbilog::Debug:
      printf("DEBUG");
      break;
  }

  printf("[% 7.3f",((double)clock())/CLOCKS_PER_SEC);

  if(threadID)
  {
    printf(":%x",threadID);
  }

  if(l.category.size()>0)
  {
    printf(":%s",l.category.c_str());
  }
  else
  {
    if(strcmp(l.moduleName,"n/a"))
    {
      printf(":%s",l.moduleName);
    }
  }
  
  printf("] ");
  
  std::string msg=l.message;
  
  int s=msg.size();
  
  if(s>0)
    if(msg[s-1]=='\n')
      msg = msg.substr(0,s-1);
  
  printf("%s\n",msg.c_str());  

}

void mbilog::backendCout::formatFull(const LogMessage &l,int threadID)
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