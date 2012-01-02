/*=========================================================================

Program:   mbilog - logging for mitk / BlueBerry

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <list>
#include <ctime>
#include <iomanip>

#include "mbilogBackendCout.h"

static bool g_init=false;

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

void mbilog::BackendCout::AppendTimeStamp(std::ostream& out)
{
  time_t rawtime = time(NULL);
  std::string timestring( ctime(&rawtime) );
  timestring.replace( timestring.length() -1, 1," "); // replace \n by " " (separates date/time from following output of relative time since start)

  std::locale C("C");
  std::locale originalLocale = out.getloc();
  out.imbue(C);

  out << timestring;

  out.imbue( originalLocale );

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

  out << c_open;

  if (!g_init)
  {
    g_init = true;
    AppendTimeStamp(out);
    out << std::endl;
  }

  std::locale C("C");
  std::locale originalLocale = out.getloc();
  out.imbue(C);

  out << std::fixed << std::setprecision(3) << ((double)std::clock())/CLOCKS_PER_SEC;

  out.imbue( originalLocale );

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

  out << "|";

  AppendTimeStamp(out);

  out << "|";


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

  out << l.message << std::endl;
}

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

#ifdef USE_WIN32COLOREDCONSOLE

void mbilog::BackendCout::FormatSmartWindows(const mbilog::LogMessage &l,int /*threadID*/)
{
  int colorNormal = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;

  int lastColor = colorNormal;
  #define ChangeColor( _col ) { int col=(_col); if(lastColor != (col)) { SetConsoleTextAttribute(g_hConsole, (col) ); lastColor=(col); } }

  int colorTime = FOREGROUND_GREEN;
  int colorText = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
  int colorCat = FOREGROUND_BLUE | FOREGROUND_RED;
  bool showColon = true;
  bool forceCat = false;

  if(!g_init)
  {
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    g_init=true;

    std::string title = "mbilog";

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

    // Give out start time
    ChangeColor( colorTime );
    AppendTimeStamp(std::cout);
    std::cout << std::endl;
  }

  switch(l.level)
  {
    case mbilog::Info:
      break;

    case mbilog::Warn:
      colorTime = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY;
      //colorText = FOREGROUND_RED|FOREGROUND_GREEN;
      colorCat = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
      showColon = false;
      forceCat = true;
      break;

    case mbilog::Error:
      colorTime = FOREGROUND_RED|FOREGROUND_INTENSITY;
      //colorText = FOREGROUND_RED;
      colorCat = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
      showColon = false;
      forceCat = true;
      break;

    case mbilog::Fatal:
      colorTime = FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      //colorText = FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      colorCat = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
      showColon = false;
      forceCat = true;
      break;

    case mbilog::Debug:
      colorTime = FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      //colorText |= FOREGROUND_INTENSITY;
      showColon = false;
      break;
  }

  ChangeColor( colorTime );

  std::locale C("C");
  std::locale originalLocale = std::cout.getloc();
  std::cout.imbue(C);

  std::cout << std::fixed << std::setprecision(2) << ((double)std::clock())/CLOCKS_PER_SEC << " ";

  std::cout.imbue( originalLocale );


  // category
  {
    AutoCategorize ac(l);
    std::string pre=ac.GetPrefix();
    std::string cat=ac.GetCategory();

    cat = pre + cat;

    if(cat.empty())
      cat = l.category;

    if(!cat.empty())
    {
      ChangeColor( colorCat );
//      static std::string lastCat;
  //    if(forceCat||lastCat.compare(cat))
      {
        std::cout << cat << std::flush;
    //    lastCat = cat;
      }
    //  else
    //    std::cout << "..." << std::flush;

      if(showColon)
      {
        ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
        std::cout << ": " << std::flush;
      }
      else
        std::cout << " ";
    }
  }

  switch(l.level)
  {
    case mbilog::Info:
      break;

    case mbilog::Warn:
      ChangeColor( colorTime );
      std::cout << "WARNING" << std::flush;
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
      std::cout << ": " << std::flush;
      break;

    case mbilog::Error:
      ChangeColor( colorTime );
      std::cout << "ERROR" << std::flush;
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
      std::cout << ": " << std::flush;
      break;

    case mbilog::Fatal:
      ChangeColor( colorTime );
      std::cout << "FATAL" << std::flush;
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
      std::cout << ": " << std::flush;
      break;

    case mbilog::Debug:
      ChangeColor( colorTime );
      std::cout << "DBG" << std::flush;
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
      std::cout << ": " << std::flush;
      break;
  }

  ChangeColor( colorText );
  std::cout << l.message << std::endl;

  ChangeColor( colorNormal );
}

#endif



