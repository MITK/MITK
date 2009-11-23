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

    std::string current,category;

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

    std::string simplify(std::string x)
    {
      static char *replace[] =
      {
        ".cpp",                "",
        ".cxx",                "",
        ".txx",                "",
        ".h",                  "",
        ".hpp",                "",
        ".hxx",                "",
        ".c",                  "",
        
        "org.opencherry.",     "",
        "org.mitk.gui.qt.",    "",
        "org.mitk.",           "",

        "qmitk",               "",
        "mitk",                "",
        "cherry",              "",
        "itk",                 "",
        "vtk",                 "",
        "qt",                  "",
       
        "object",              "obj",
        "factory",             "fac",
        "classes",             "cls",
        "plugin",              "plg",
        "widget",              "wid",
        "interface",           "itf",
        "service",             "svc",
        "register",            "reg",
        "perspective",         "prs",

        "contour",             "",
        "tools",               "",
        "tool",                "",
      
        "application",        "app",
        "calculate",          "calc",
        "subtract",           "sub",
        "region",             "reg",
        "tumor",              "tum",
        "growing",            "grow",
        "segmentation",       "seg",
        "statistics",         "stat",
    
        "imaging",            "img",
        "image",              "img",
    
        "diffusion",          "dif",
        "registration",       "reg",
        "navigation",         "nav",

        "generation",         "gen",
        "generator",          "gen",

        "paintbrush",         "pb",
        "volumetry",          "vol",
        "volume",             "vol",
        "mapper",             "map",
        "filter",             "flt",
        "surface",            "sfc",
        "point",              "pnt",
        "organ",              "org",
        "multiple",           "mul",
   
        "corrector",          "cor",
        "correction",         "cor",
    
        "sandbox",            "sb",
        "texture",            "tex",
        "opengl",             "ogl",
        "vessel",             "vsl",
        "value",              "val",
        "analysis",           "ana",

        "patient",            "pat",
        "body",               "body",
        "diagnosis",          "diag",
        "mesh",               "mesh",
        "radial",             "rad",
        "simple",             "sim",
        "algorithms",         "alg",
        "controllers",        "con",
        "control",            "con",
        "interactive",        "ia",
        "interactions",       "ia",
      
        "processing",         "pro", 
        "process",            "pro", 
      
        "rendering",          "rnd", 
        "renderer",           "rnd", 
        "render",             "rnd", 

        "datamanagement",     "data",
        "management",         "mng",
        "manager",            "mng", 
        "data",               "data",

        "anatomy",            "ana", 
        "neuro",              "neo", 
        "automatic",          "auto", 
    
        "optimizer",          "opt", 
        "optimize",           "opt", 

        "binary",             "bin", 
        "liver",              "liv", 
        "lymph",              "lym", 
        "node",               "node", 
        "tree",               "tree", 
        "homogeneous",        "homogen", 
        "threshold",          "tsh", 

        "shapebased",         "shp", 
        "based",              "bsd",
        "shape",              "shp", 

        "model",              "mdl", 
        "extension",          "ext", 
        "activator",          "act", 
        "view",               "view", 
        
        "finder",             "fnd",
        "indexer",            "idx",
        "rapid",              "rpd",

        "planning",           "plan",         
        "planner",            "plan",
        "plane",              "pln",
        "plan",               "plan",
        
        "workbench",          "wrkbnch",
        "common",             "com",
        "resection",          "rsc",
        "translation",        "trl",
        "rotation",           "rot",
        "deformation",        "dfrm",
        "shader",             "shd",
        "repository",         "rep",
        "initializer",        "ini",
        "dialog",             "dlg",
        "download",           "down",
        "upload",             "up",
        "core",               "core",
        "manual",             "man",
        "leaf",               "leaf",
        
        "2d",                 "2d",
        "3d",                 "3d",
        ".",                  "." 
        
      };
      
      bool redo;
      
      std::string lft(""),rgt("");
      
      do
      {
        redo=false;

        for(int r=0; r < sizeof(replace)/sizeof(char*); r+=2)
        {
          int s=strlen(replace[r]);
          int xs=x.size();

          if(xs==s)
          {
            if( replace[r+1][0] || !lft.empty() || !rgt.empty() )
              if(x.compare(replace[r])==0)
                x=replace[r+1];
          }
          else if(xs>s)
          {
            if(strncmp(replace[r],&x.c_str()[xs-s],s)==0)
            {
              std::string rp = replace[r+1];
              if(!rp.empty()) rp[0]=toupper(rp[0]);
              x = x.substr(0,xs-s);
              rgt = rp + rgt;
              redo=true;
            }
            else if(strncmp(replace[r],x.c_str(),s)==0)
            {
              std::string rp = replace[r+1];
              if(!rp.empty()) rp[0]=toupper(rp[0]);
              x=x.substr(s,xs-s);
              lft = lft + rp; 
              redo=true;
            }
          }
        }
      }
      while(redo);
      
      x[0]=toupper(x[0]);
      
      x=lft+x+rgt;
      
      x[0]=tolower(x[0]);

      return x;
    }
    
    std::string concat(std::string a,std::string b,bool opt)
    {
      int as=a.size();
      int bs=b.size();
      if(opt && as<=bs)
      {
        if(as==bs && a.compare(b)==0)
          return a;
        
        if(strncmp(a.c_str(),b.c_str(),as)==0)
        {
          b=b.substr(as,bs-as);
          b[0]=tolower(b[0]);
        }
      }
      
      return a+"."+b;
    }
    
    bool search2p2(char *a,char *b,bool optimize=true)
    {
      int size = path.size() - 3;
      for(int r=0;r<size;r++)
        if(path[r].compare(a)==0 && path[r+1].compare(b)==0)
        {
          pos = r+2;
          category = concat(simplify(path[pos]),simplify(path[path.size()-1]),optimize);
          return true;
        }
      return false;
    }

    bool search2p1(char *a,char *b)
    {
      int size = path.size() - 2;
      for(int r=0;r<size;r++)
        if(path[r].compare(a)==0 && path[r+1].compare(b)==0)
        {
          pos = r+2;
          category = simplify(path[path.size()-1]);
          return true;
        }
      return false;
    }

    bool search1p2(char *a,bool optimize=true)
    {
      int size = path.size() - 2;
      for(int r=0;r<size;r++)
        if(path[r].compare(a)==0)
        {
          pos = r+1;
          category = concat(simplify(path[pos]),simplify(path[path.size()-1]),optimize);
          return true;
        }
      return false;
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
    
    std::string GetPrefix()
    {
      category="";
      if(search2p2("mbi-sb","core",false))      return "sb.";
      if(search2p1("mbi-sb","q4mitk"))          return "sb.ui.";
      if(search2p2("mbi","applications"))       return "sb.app.";
      if(search2p2("mbi-sb","q4applications"))  return "sb.app.";
      if(search2p2("mbi-sb","utilities"))       return "sb.utl.";
      if(search2p2("mbi-sb","bundles"))         return "sb.bun.";   
      if(search2p2("mbi-sb","bundlesqt"))       return "sb.bun.";   
      if(search2p2("mbi","modules"))            return "sb.mod.";

      if(search2p2("mbi-qm","core",false))      return "qm.";
      if(search2p2("mbi-qm","utilities"))       return "qm.utl.";

      if(search2p2("modules","mitkext",false))  return "ext.";
      if(search2p1("modules","qmitkext"))       return "ext.ui.";
      if(search2p2("modules","bundles"))        return "ext.bun.";       

      if(search2p2("opencherry","bundles"))     return "blueberry.";     

      if(search2p2("core","code",false))        return "core.";
      if(search2p1("coreui","qmitk"))           return "core.ui.";
      if(search2p2("coreui","bundles"))         return "core.bun.";
       
      // following must come last:      
      if(search1p2("modules"))                  return "core.mod.";
      if(search1p2("utilities"))                return "core.utl.";
      if(search1p2("applications"))             return "core.app.";
      
      return "";
    }
    
    std::string GetCategory()
    {
      return category;
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

  int colorTime = FOREGROUND_GREEN;
  int colorText = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
  int colorCat = FOREGROUND_BLUE | FOREGROUND_RED;
  bool showColon = true;
  
  switch(l.level)
  {
    case mbilog::Info:
      break;

    case mbilog::Warn:
      colorTime = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY;
      colorText = FOREGROUND_RED|FOREGROUND_GREEN;
      colorCat = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
      showColon = false;
      break;

    case mbilog::Error:
      colorTime = FOREGROUND_RED|FOREGROUND_INTENSITY;
      colorText = FOREGROUND_RED;
      colorCat = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
      showColon = false;
      break;

    case mbilog::Fatal:
      colorTime = FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      colorText = FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      colorCat = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
      showColon = false;
      break;

    case mbilog::Debug:
      colorTime = FOREGROUND_BLUE|FOREGROUND_INTENSITY;
      colorText |= FOREGROUND_INTENSITY;
      showColon = false;
      break;
  }

  ChangeColor( colorTime );
  std::cout << std::fixed << std::setprecision(2) << ((double)std::clock())/CLOCKS_PER_SEC << " " << std::flush;
  
  {
    AutoCategorize ac(l);
    std::string pre=ac.GetPrefix();
    std::string cat=ac.GetCategory();
    if( (!pre.empty()) || (!cat.empty()) )
    {
      if( !pre.empty() )
      {
        ChangeColor( colorCat );
        std::cout << pre << std::flush;
      }
      if( !cat.empty() )
      {
        ChangeColor( colorCat );
        std::cout << cat << std::flush;
      }
    
      if(showColon)
      {
        ChangeColor( FOREGROUND_BLUE | FOREGROUND_INTENSITY );
        std::cout << ": " << std::flush;
      }
      else
        std::cout << " ";
    }
    else if(!l.category.empty())
    {
      ChangeColor( colorCat );
      std::cout << l.category << std::flush;
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
