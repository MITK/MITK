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

static std::list<mbilog::BackendBase*> backends;


namespace mbilog {
static const std::string NA_STRING = "n/a";
}

void mbilog::RegisterBackend(mbilog::BackendBase* backend)
{
  backends.push_back(backend);
}

void mbilog::UnregisterBackend(mbilog::BackendBase* backend)
{
  backends.remove(backend);
}

void mbilog::DistributeToBackends(mbilog::LogMessage &l)
{

  // Crop Message
  {
    std::string::size_type i = l.message.find_last_not_of(" \t\f\v\n\r");
    l.message = (i != std::string::npos) ? l.message.substr(0, i+1) : "";
  }

  if(backends.empty())
  {
    mbilog::BackendCout::FormatSmart(l);
    return;
  }

  std::list<mbilog::BackendBase*>::iterator i;

  for(i = backends.begin(); i != backends.end(); i++)
    (*i)->ProcessMessage(l);
}


#ifdef USE_WIN32COLOREDCONSOLE

static HANDLE g_hConsole;

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

        "org.blueberry.",      "",
        "org.mitk.gui.qt.",    "",
        "org.mitk.",           "",

        "qmitk",               "",
        "mitk",                "",
        "berry",               "",
        "itk",                 "",
        "vtk",                 "",
        "qt",                  "",

        "object",              "obj",
        "factory",             "fac",
        "classes",             "cls",
        "plugin",              "plg",
        "widget",              "wdgt",
        "interface",           "itf",
        "service",             "svc",
        "register",            "reg",
        "perspective",         "prs",

        "assessor",            "ase",

        "atrophy",             "atr",

        "bias",                "bias",

        "field",               "fld",

        "multi",               "mlt",

        "contour",             "cntr",
        "tools",               "tls",
        "tool",                "tl",

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

        "vector",             "vec",
        "gradient",           "grad",
        "flow",               "flow",

        "paint",              "pnt",
        "brush",              "brsh",
        "volumetry",          "vol",
        "volume",             "vol",
        "mapper",             "map",
        "filter",             "flt",
        "surface",            "sfc",
        "point",              "pnt",
        "organ",              "org",
        "multiple",           "mlt",

        "corrector",          "cor",
        "correction",         "cor",

        "batch",              "bat",

        "window",             "wnd",

        "advisor",            "adv",

        "editor",             "edt",

        "material",           "mat",

        "visualization",      "vis",

        "measurement",        "mes",

        "scene",              "scn",

        "serialization",      "ser",
        "deserializer",       "dser",
        "serializer",         "ser",

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
        "simple",             "smp",
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
        "homogeneous",        "hmgn",
        "threshold",          "tsh",

     //   "shapebased",         "shp",
        "based",              "bsd",
        "shape",              "shp",

        "model",              "mdl",
        "extension",          "ext",
        "activator",          "act",

        "dicom",              "dicom",

        "browser",            "brwr",

        "viewer",             "view",
        "view",               "view",

        "finder",             "fnd",

        "indexer",            "idx",
        "index",              "idx",

        "rapid",              "rpd",

        "gui",                "gui",

        "slices",             "slc",
        "slice",              "slc",

        "about",              "abt",

        "interpolator",       "inp",

        "switcher",           "swh",

        "planning",           "plan",
        "planner",            "plan",
        "plane",              "pln",
        "plan",               "plan",

        "workbench",          "wrkbnc",
        "common",             "com",
        "resection",          "rsc",
        "translation",        "trnsl",
        "rotation",           "rot",
        "deformation",        "dfrm",
        "shader",             "shd",
        "repository",         "rep",
        "initializer",        "init",
        "dialog",             "dlg",
        "download",           "down",
        "upload",             "up",
        "core",               "core",
        "manual",             "man",
        "leaf",               "leaf",
        "internal",           "int",
        "external",           "ext",
        "platform",           "pltfm",
        "method",             "mthd",
        "pyramidal",          "prmdl",
        "tracking",           "trck",
        "track",              "trck",

        "bspline",            "bspl",
        "spline",             "spl",

        "create",             "crt",
        "erase",              "ers",

        "auto",               "auto",
        "crop",               "crop",
        "file",               "file",
        "io",                 "io",

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
          int s  = static_cast<int>( strlen(replace[r]) );
          int xs = static_cast<int>( x.size() );

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
      int as = static_cast<int>( a.size() );
      int bs = static_cast<int>( b.size() );
      if(opt && as <= bs)
      {
        if (as==bs && a.compare(b)==0)
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
      int size = static_cast<int>( path.size() ) - 3;
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
      int size = static_cast<int>( path.size() ) - 2;
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
      int size = static_cast<int>( path.size() ) - 2;
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
      int size = static_cast<int>( strlen(l.filePath) );

      current = "";

      for(int r = 0;r<size;r++)
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
      if(search2p2("mbi-sb","utilities"))       return "sb.util.";
      if(search2p2("mbi-sb","bundles"))         return "sb.bun.";
      if(search2p2("mbi-sb","bundlesqt"))       return "sb.bun.";
      if(search2p2("mbi","modules"))            return "sb.mod.";

      if(search2p2("mbi-qm","core",false))      return "qm.";
      if(search2p2("mbi-qm","utilities"))       return "qm.util.";

      if(search2p2("modules","mitkext",false))  return "ext.";
      if(search2p1("modules","qmitkext"))       return "ext.ui.";
      if(search2p2("modules","bundles"))        return "ext.bun.";

      if(search2p2("blueberry","bundles"))     return "blueberry.";

      if(search2p2("core","code",false))        return "core.";
      if(search2p1("coreui","qmitk"))           return "core.ui.";
      if(search2p2("coreui","bundles"))         return "core.bun.";

      // following must come last:
      if(search1p2("modules"))                  return "core.mod.";
      if(search1p2("utilities"))                return "core.util.";
      if(search1p2("applications"))             return "core.app.";

      return "";
    }

    std::string GetCategory()
    {
      return category;
    }

};




#endif