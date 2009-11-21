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
#include <string>
#include <iomanip>
#include <vector>

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

class AutoCategorize
{
  protected:

    std::vector<std::string> path;

    std::string current;

    int pos;

    void flush()
    {
      if(current.size()>0)
      {
        if(current.compare("..")==0)
        {
          if(path.size()>0)
            path.pop_back();
        }
        else
        {
          path.push_back(current);
        }
        current="";
      }
    }

    bool search2p1(char *a,char *b)
    {
      int size = path.size() - 2;
      for(int r=0;r<size;r++)
        if(path[r].compare(a)==0 && path[r+1].compare(b)==0)
        {
          pos = r+2;
          return true;
        }
      return false;
    }

    bool search1p1(char *a)
    {
      int size = path.size() - 1;
      for(int r=0;r<size;r++)
        if(path[r].compare(a)==0)
        {
          pos = r+1;
          return true;
        }
      return false;
    }
    
    std::string stripBundle(std::string x)
    {
      static char *strip[] =
      {
        "org.opencherry.",
        "org.mitk.gui.qt.",
        "org.mitk."
      };
      
      int xs=x.size();
      
      for(int r=0; r < sizeof(strip)/sizeof(char*); r++)
      {
        int s=strlen(strip[r]);
        if(xs>s)
          if(strncmp(strip[r],x.c_str(),s)==0)
            return x.substr(s,xs-s);
      }
      return x;
    }

  public:
  
    AutoCategorize( const mbilog::LogMessage &l )
    {
      int size =strlen(l.filePath);

      current="";

      for(int r=0;r<size;r++)
      {
        char c=l.filePath[r];
        if(c=='\\' || c=='/')
          flush();
        else 
          current+=tolower(c);
      }

      flush();
    }
    
    std::string GetCategory()
    {
      if(search2p1("modules","mitkext"))
        return "ext." + path[pos];

      if(search2p1("modules","qmitkext"))
        return "ext.ui";

      if(search2p1("mbi-sb","core"))
        return "sb." + path[pos];
    
      if(search2p1("mbi-sb","q4mitk"))
        return "sb.ui";
    
      if(search2p1("mbi-sb","q4applications"))
        return "sb.app." + path[pos];

      if(search2p1("mbi-sb","utilities"))
        return "sb.util." + path[pos];
    
      if(search2p1("mbi-sb","bundles"))
        return "sb.bun." + stripBundle(path[pos]);   
    
      if(search2p1("mbi-sb","bundlesqt"))
        return "sb.bun." + stripBundle(path[pos]);   
    
      if(search2p1("mbi-qm","core"))
        return "qm." + path[pos];
        
      if(search2p1("mbi-qm","utilities"))
        return "qm.util." + path[pos];
    
      if(search2p1("mbi","modules"))
        return "sb.mod." + path[pos];
        
      if(search2p1("core","code"))
        return path[pos];
    
      if(search2p1("coreui","bundles"))
        return "bun." + stripBundle(path[pos]);       
    
      if(search2p1("modules","bundles"))
        return "ext.bun." + stripBundle(path[pos]);       
    
      if(search2p1("coreui","qmitk"))
        return "ui";
    
      if(search2p1("opencherry","bundles"))
        return "blueberry." + stripBundle(path[pos]);     
    
      if(search1p1("modules"))
        return "mod." + path[pos];
    
      if(search1p1("utilities"))
        return "util." + path[pos];
    
      return "";
      
    /*
      std::string result("");
      
      for (std::vector<std::string>::iterator it = path.begin(); it != path.end(); ++it)
      {
        if(result.empty())
          result+=*it;
        else
          result+="/"+*it;
      }
      
      return result;
    */
    }
    
};


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
  
  {
    AutoCategorize ac(l);
    std::string cat=ac.GetCategory();
    if(!cat.empty())
    {
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
      std::cout << "{" << std::flush;
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY );
      std::cout << cat << std::flush;
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
      std::cout << "} " << std::flush;
    }
    else if(!l.category.empty())
    {
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
      std::cout << "(" << std::flush;
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_RED );
      std::cout << l.category << std::flush;
      ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
      std::cout << ") " << std::flush;
    }
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
