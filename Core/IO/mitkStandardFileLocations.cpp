#include <mitkStandardFileLocations.h>

#include <mitkConfig.h>
#include <itkObject.h>
#include <itkMacro.h>
#include <itksys/SystemTools.hxx>


const std::string mitk::StandardFileLocations::FindFile(const char* filename, const char* pathInSourceDir)
{
  const char* mitkConf = itksys::SystemTools::GetEnv("MITKCONF");
  std::string xmlFileName;

  if (mitkConf!=NULL)
  {
    // 1. look for MITKCONF environment variable
    xmlFileName = mitkConf;

    xmlFileName += "/";
    xmlFileName = itksys::SystemTools::ConvertToOutputPath(xmlFileName.c_str());
    xmlFileName += filename;

    if(itksys::SystemTools::FileExists(xmlFileName.c_str())) return xmlFileName;
  }

  // 2. use .mitk-subdirectory in home directory of the user
  std::string homeDirectory;
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* homeDrive = itksys::SystemTools::GetEnv("HOMEDRIVE");
  const char* homePath = itksys::SystemTools::GetEnv("HOMEPATH");
  if((homeDrive==NULL) || (homePath==NULL))
  {
    itkGenericOutputMacro( << "Environment variables HOMEDRIVE and/or HOMEPATH not set" <<
      ". Using current working directory as home directory: " << itksys::SystemTools::GetCurrentWorkingDirectory());
    homeDirectory = itksys::SystemTools::GetCurrentWorkingDirectory();
  }
  else
  {
    homeDirectory = homeDrive;
    homeDirectory +=homePath;

    // homeDirectory must not have a slash at the end, otherwise a path like "C:\/.mitk/" might occur.
    if(homeDirectory.find_last_of("\\")+1 == homeDirectory.size() || homeDirectory.find_last_of("/")+1 == homeDirectory.size())
     homeDirectory.erase(homeDirectory.size()-1,homeDirectory.size());
  }
  if(itksys::SystemTools::FileExists(homeDirectory.c_str())==false)
  {
    itkGenericOutputMacro( << "Could not find home directory at " << homeDirectory << 
      ". Using current working directory as home directory: " << itksys::SystemTools::GetCurrentWorkingDirectory());
    homeDirectory = itksys::SystemTools::GetCurrentWorkingDirectory();
  }
#else
  const char* home = itksys::SystemTools::GetEnv("HOME");
  if(home==NULL)
  {
    itkGenericOutputMacro( << "Environment variable HOME not set" <<
      ". Using current working directory as home directory: " << itksys::SystemTools::GetCurrentWorkingDirectory());
    homeDirectory = itksys::SystemTools::GetCurrentWorkingDirectory();
  }
  else
    homeDirectory = home;
  if(itksys::SystemTools::FileExists(homeDirectory.c_str())==false)
  {
    itkGenericOutputMacro( << "Could not find home directory at " << homeDirectory << 
      ". Using current working directory as home directory: " << itksys::SystemTools::GetCurrentWorkingDirectory());
    homeDirectory = itksys::SystemTools::GetCurrentWorkingDirectory();
  }
#endif // defined(_WIN32) && !defined(__CYGWIN__)
  
  xmlFileName = homeDirectory;
  xmlFileName += "/.mitk/";
  xmlFileName += filename;

  if(itksys::SystemTools::FileExists(xmlFileName.c_str())) return xmlFileName;

  // 3. look in the current working directory
  xmlFileName = filename;

  if(itksys::SystemTools::FileExists(xmlFileName.c_str())) return xmlFileName;

  // 4. use a source tree location from compile time
  xmlFileName = MITK_ROOT;
  if (pathInSourceDir)
  {
    xmlFileName += pathInSourceDir;
  }
  xmlFileName += '/';
  xmlFileName += filename;
  
  if(itksys::SystemTools::FileExists(xmlFileName.c_str())) return xmlFileName;

  return std::string("");
}

const std::string mitk::StandardFileLocations::GetOptionDirectory()
{
  const char* mitkoptions = itksys::SystemTools::GetEnv("MITKOPTIONS");
  std::string optionsDirectory;

  if (mitkoptions!=NULL)
  {
    // 1. look for MITKOPTIONS environment variable
    optionsDirectory = mitkoptions;
    optionsDirectory += "/";
  }
  else
  {
    // 2. use .mitk-subdirectory in home directory of the user
    std::string homeDirectory;
#if defined(_WIN32) && !defined(__CYGWIN__)
    const char* homeDrive = itksys::SystemTools::GetEnv("HOMEDRIVE");
    const char* homePath = itksys::SystemTools::GetEnv("HOMEPATH");
    if((homeDrive==NULL) || (homePath==NULL))
    {
      itkGenericOutputMacro( << "Environment variables HOMEDRIVE and/or HOMEPATH not set" <<
        ". Using current working directory as home directory: " << itksys::SystemTools::GetCurrentWorkingDirectory());
      homeDirectory = itksys::SystemTools::GetCurrentWorkingDirectory();
    }
    else
    {
      homeDirectory = homeDrive;
      homeDirectory +=homePath;
    }
    if(itksys::SystemTools::FileExists(homeDirectory.c_str())==false)
    {
      itkGenericOutputMacro( << "Could not find home directory at " << homeDirectory << 
        ". Using current working directory as home directory: " << itksys::SystemTools::GetCurrentWorkingDirectory());
      homeDirectory = itksys::SystemTools::GetCurrentWorkingDirectory();
    }
#else
    const char* home = itksys::SystemTools::GetEnv("HOME");
    if(home==NULL)
    {
      itkGenericOutputMacro( << "Environment variable HOME not set" <<
        ". Using current working directory as home directory: " << itksys::SystemTools::GetCurrentWorkingDirectory());
      homeDirectory = itksys::SystemTools::GetCurrentWorkingDirectory();
    }
    else
      homeDirectory = home;
    if(itksys::SystemTools::FileExists(homeDirectory.c_str())==false)
    {
      itkGenericOutputMacro( << "Could not find home directory at " << homeDirectory << 
        ". Using current working directory as home directory: " << itksys::SystemTools::GetCurrentWorkingDirectory());
      homeDirectory = itksys::SystemTools::GetCurrentWorkingDirectory();
    }
#endif // defined(_WIN32) && !defined(__CYGWIN__)
   
    optionsDirectory = homeDirectory;
    optionsDirectory += "/.mitk";
  }

  optionsDirectory = itksys::SystemTools::ConvertToOutputPath(optionsDirectory.c_str());
  if(itksys::SystemTools::CountChar(optionsDirectory.c_str(),'"') > 0)
  {
    char * unquoted = itksys::SystemTools::RemoveChars(optionsDirectory.c_str(),"\"");
    optionsDirectory = unquoted;
    delete [] unquoted;
  }

  if(itksys::SystemTools::MakeDirectory(optionsDirectory.c_str())==false)
  {
    itkGenericExceptionMacro( << "Could not create .mitk directory at " << optionsDirectory );
  }
  return optionsDirectory;
}
