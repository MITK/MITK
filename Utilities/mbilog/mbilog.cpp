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

static HANDLE g_hConsole;
static bool g_init=false;

static void FormatSmartWindows(const mbilog::LogMessage &l,int /*threadID*/)
{
  int colorNormal = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;

  int lastColor = colorNormal;
  #define ChangeColor( _col ) { int col=(_col); if(lastColor != (col)) { SetConsoleTextAttribute(g_hConsole, (col) ); lastColor=(col); } }

  if(!g_init)
  {
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    g_init=true;

    std::string title = "MITK - Log";
    
    SetConsoleTitle( title.c_str() );
    
    /* Title rendering
    ChangeColor( FOREGROUND_GREEN|FOREGROUND_BLUE|BACKGROUND_BLUE );
    std::cout << " <<< " << std::flush;
    ChangeColor( FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY|BACKGROUND_BLUE );
    std::cout << title << std::flush;
    ChangeColor( FOREGROUND_GREEN|FOREGROUND_BLUE|BACKGROUND_BLUE );
    std::cout << " >>> " << std::flush;
    ChangeColor( colorNormal );
    std::cout << std::endl;
    */
  }

  char c_open='[';
  char c_close=']';

  int colorTime = FOREGROUND_GREEN;
  int colorText = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
  
  switch(l.level)
  {
    case mbilog::Info:
      break;

    case mbilog::Warn:
      c_open='!';
      c_close='!';
      colorTime = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY;
      colorText = FOREGROUND_RED|FOREGROUND_GREEN;
      break;

    case mbilog::Error:
      c_open='#';
      c_close='#';
      colorTime = FOREGROUND_RED|FOREGROUND_INTENSITY;
      colorText = FOREGROUND_RED;
      break;

    case mbilog::Fatal:
      c_open='*';
      c_close='*';
      colorTime = FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      colorText = FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      break;

    case mbilog::Debug:
      c_open='(';
      c_close=')';
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
    std::cout << "{" << std::flush;
    ChangeColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY );
    std::cout << l.category << std::flush;
    ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
    std::cout << "} " << std::flush;
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
