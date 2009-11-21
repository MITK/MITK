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

#ifdef _WIN32
 #define USE_WIN32COLOREDCONSOLE
 #include <windows.h>
#endif

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

void mbilog::DistributeToBackends(mbilog::LogMessage &l)
{

  // Crop Message
  {
    std::size_t i = l.message.find_last_not_of(" \t\f\v\n\r");
    l.message = (i != std::string::npos) ? l.message.substr(0, i+1) : "";
  }
  
  if(backends.empty())
  {
    mbilog::BackendCout::FormatSmart(l);
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

void mbilog::BackendCout::FormatSmart(std::ostream &out, const LogMessage &l,int /*threadID*/)
{
  char c_open='[';
  char c_close=']';

  switch(l.level)
  {
    case mbilog::Info:
      break;

    case mbilog::Warn:
      c_open='!';
      c_close='!';
      break;

    case mbilog::Error:
      c_open='#';
      c_close='#';
      break;

    case mbilog::Fatal:
      c_open='*';
      c_close='*';
      break;

    case mbilog::Debug:
      c_open='{';
      c_close='}';
      break;
  }

  out << c_open << std::fixed << std::setw(6) << std::setprecision(2) << ((double)std::clock())/CLOCKS_PER_SEC;

  out << c_close << " ";
  
  if(!l.category.empty())
  {
    out << "[" << l.category << "] ";
  }
 
  switch(l.level)
  {
    case mbilog::Info:
      break;

    case mbilog::Warn:
      out << "WARNING: ";
      break;

    case mbilog::Error:
      out << "ERROR: ";
      break;

    case mbilog::Fatal:
      out << "FATAL: ";
      break;

    case mbilog::Debug:
      out << "DEBUG: ";
      break;
  }

  out << l.message << std::endl;
}

void mbilog::BackendCout::FormatFull(std::ostream &out,const LogMessage &l,int threadID)
{
  switch(l.level)
  {
    case mbilog::Info:
      out << "INFO";
      break;

    case mbilog::Warn:
      out << "WARN";
      break;

    case mbilog::Error:
      out << "ERROR";
      break;

    case mbilog::Fatal:
      out << "FATAL";
      break;

    case mbilog::Debug:
      out << "DEBUG";
      break;
  }

  out << std::setprecision(3) << ":" << ((double)std::clock())/CLOCKS_PER_SEC;

  out << "|" << std::string(l.filePath) << "(" << l.lineNumber << ")";

  out << "|" << std::string(l.functionName);

  //if(threadID)
  {
    out << "|" << std::hex << threadID;
  }

  //if(NA_STRING != l.moduleName)
  {
    out << "|" << std::string(l.moduleName);
  }

  //if(!l.category.empty())
  {
    out << "|" << l.category;
  }


  out << "\n" << l.message << std::endl;
}

#ifdef USE_WIN32COLOREDCONSOLE

static int g_LastColor;

static void ChangeColor(int c)
{
  if(g_LastColor == c)
    return;
    
  static HANDLE hConsole;

  static bool init=false;
  if(!init)
  {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    init=true;
  }

  SetConsoleTextAttribute(hConsole,c);
  
  g_LastColor=c;  
}

static void FormatSmartWindows(const mbilog::LogMessage &l,int /*threadID*/)
{
  char c_open='[';
  char c_close=']';

  int colorNormal = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;

  int colorTime = FOREGROUND_GREEN;
  int colorText = colorNormal;
  
  g_LastColor = colorNormal;

  switch(l.level)
  {
    case mbilog::Info:
      break;

    case mbilog::Warn:
      c_open='!';
      c_close='!';
      colorTime = FOREGROUND_RED|FOREGROUND_GREEN;
      colorText |= FOREGROUND_INTENSITY;
      break;

    case mbilog::Error:
      c_open='#';
      c_close='#';
      colorTime = FOREGROUND_RED|FOREGROUND_INTENSITY;
      colorText |= FOREGROUND_INTENSITY;
      break;

    case mbilog::Fatal:
      c_open='*';
      c_close='*';
      colorTime = FOREGROUND_RED|FOREGROUND_INTENSITY;
      colorText |= FOREGROUND_INTENSITY;
      break;

    case mbilog::Debug:
      c_open='{';
      c_close='}';
      colorTime = FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      colorText |= FOREGROUND_INTENSITY;
      break;
  }

  ChangeColor( colorTime );
  std::cout << c_open << std::flush;
  ChangeColor( colorTime | FOREGROUND_INTENSITY);
  std::cout << std::fixed << std::setw(6) << std::setprecision(2) << ((double)std::clock())/CLOCKS_PER_SEC << std::flush;
  ChangeColor( colorTime );
  std::cout << c_close << " " << std::flush;

  if(!l.category.empty())
  {
    ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
    std::cout << "[" << std::flush;
    ChangeColor( FOREGROUND_BLUE | FOREGROUND_GREEN );
    std::cout << l.category << std::flush;
    ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
    std::cout << "] " << std::flush;
  }
 
  switch(l.level)
  {
    case mbilog::Info:
      break;

    case mbilog::Warn:
      ChangeColor( colorTime );
      std::cout << "WARNING: " << std::flush;
      break;

    case mbilog::Error:
       ChangeColor( colorTime );
     std::cout << "ERROR: " << std::flush;
      break;

    case mbilog::Fatal:
      ChangeColor( colorTime );
      std::cout << "FATAL: " << std::flush;
      break;

    case mbilog::Debug:
      ChangeColor( colorTime );
      std::cout << "DEBUG: " << std::flush;
      break;
  }
      
  ChangeColor( colorText );

  std::cout << l.message << std::endl;
  
  ChangeColor( colorNormal );

}

#endif

void mbilog::BackendCout::FormatSmart(const LogMessage &l,int threadID)
{
#ifdef USE_WIN32COLOREDCONSOLE
  FormatSmartWindows(l,threadID);
#else
  mbilog::BackendCout::FormatSmart(std::cout,l,threadID);
#endif
}

void mbilog::BackendCout::FormatFull(const LogMessage &l,int threadID)
{
  mbilog::BackendCout::FormatFull(std::cout,l,threadID);
}
