#include <mitkStandardFileLocations.h>

#include <mitkConfig.h>
#include <itkObject.h>
#include <itkMacro.h>
#include <itksys/SystemTools.hxx>

#include <algorithm>

mitk::StandardFileLocations::StandardFileLocations()
{
}

mitk::StandardFileLocations::~StandardFileLocations()
{
}


void mitk::StandardFileLocations::AddDirectoryForSearch(const char * dir, bool insertInFrontOfSearchList)
{
  std::string directory = dir;
    
  // Do nothing if directory is already included into search list
  FileSearchVectorType::iterator iter;
  if(mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.size() > 0)
  {
    iter = std::find(mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.begin(),
      mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.end(),std::string(dir));
    if ( iter != mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.end() )
      return;
  }
  // insert dir into queue
  if(insertInFrontOfSearchList)
  {
    FileSearchVectorType::iterator it = mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.begin();
    mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.insert(it,std::string(dir));
  }
  else
    mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.push_back(std::string(dir));
}

void mitk::StandardFileLocations::RemoveDirectoryForSearch(const char * dir)
{
  FileSearchVectorType::iterator it;
  // background layers
  if(mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.size() > 0)
  {
    it = std::find(mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.begin(),
                   mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.end(),std::string(dir));
    if(it != mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.end())
    {
      mitk::StandardFileLocations::GetInstance()->m_SearchDirectories.erase(it);
      return;
    }
  }
}

std::string mitk::StandardFileLocations::SearchDirectoriesForFile(const char * filename)
{
  FileSearchVectorType::iterator it;
 
  for(it = m_SearchDirectories.begin(); it != m_SearchDirectories.end(); it++)
  {
    std::string currDir = (*it);
    
    // Perhaps append "/" before appending filename
    if(currDir.find_last_of("\\")+1 != currDir.size() || currDir.find_last_of("/")+1 != currDir.size())
     currDir += "/";
    
    // Append filename
    currDir += filename;
    
    // Perhaps remove "/" after filename
    if(currDir.find_last_of("\\")+1 == currDir.size() || currDir.find_last_of("/")+1 == currDir.size())
     currDir.erase(currDir.size()-1,currDir.size());

    // convert to OS dependent path schema
    currDir = itksys::SystemTools::ConvertToOutputPath(currDir.c_str());

    // On windows systems, the ConvertToOutputPath method quotes pathes that contain empty spaces.
    // These quotes are not expected by the FileExists method and therefore removed, if existing.
    if(currDir.find_last_of("\"")+1 == currDir.size())
      currDir.erase(currDir.size()-1,currDir.size());
    if(currDir.find_last_of("\"") == 0)
      currDir.erase(0,1);
    
    // Return first found path   
    if(itksys::SystemTools::FileExists(currDir.c_str())) 
      return currDir;
  }
  return std::string(""); 
}

std::string mitk::StandardFileLocations::FindFile(const char* filename, const char* pathInSourceDir)
{
  std::string directoryPath;

  // 1. look for MITKCONF environment variable
  const char* mitkConf = itksys::SystemTools::GetEnv("MITKCONF");  
  if (mitkConf!=NULL)
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(mitkConf);

  // 2. use .mitk-subdirectory in home directory of the user
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* homeDrive = itksys::SystemTools::GetEnv("HOMEDRIVE");
  const char* homePath = itksys::SystemTools::GetEnv("HOMEPATH");
  
  if((homeDrive!=NULL) || (homePath!=NULL))
  {
    directoryPath = homeDrive;
    directoryPath += homePath;
    directoryPath += "/.mitk/";
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(directoryPath.c_str());
  }
 
#else
  const char* homeDirectory = itksys::SystemTools::GetEnv("HOME");
  if(homeDirectory != NULL)
  {
    directoryPath = homeDirectory;
    directoryPath += "/.mitk/";
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(directoryPath.c_str());
  }

#endif // defined(_WIN32) && !defined(__CYGWIN__)
 
  // 3. look in the current working directory
  directoryPath = "";
  mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(directoryPath.c_str());
  
  directoryPath = itksys::SystemTools::GetCurrentWorkingDirectory();
  mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(directoryPath.c_str());

  // 4. use a source tree location from compile time
  directoryPath = MITK_ROOT;
  if (pathInSourceDir)
  {
    directoryPath += pathInSourceDir;
  }
  directoryPath += '/';
  mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(directoryPath.c_str());
  
  return mitk::StandardFileLocations::GetInstance()->SearchDirectoriesForFile(filename);
}

std::string mitk::StandardFileLocations::GetOptionDirectory()
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
