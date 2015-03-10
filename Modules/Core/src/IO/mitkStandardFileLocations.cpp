/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

mitk::StandardFileLocations* mitk::StandardFileLocations::GetInstance()
{
  static StandardFileLocations::Pointer m_Instance = 0;

  if(m_Instance.IsNull())
    m_Instance = StandardFileLocations::New();
  return m_Instance;
}


void mitk::StandardFileLocations::AddDirectoryForSearch(const char * dir, bool insertInFrontOfSearchList)
{
  std::string directory = dir;

  // Do nothing if directory is already included into search list (TODO more clever: search only once!)
  FileSearchVectorType::iterator iter;
  if(m_SearchDirectories.size() > 0)
  {
    iter = std::find(m_SearchDirectories.begin(), m_SearchDirectories.end(),std::string(dir));
    if ( iter != m_SearchDirectories.end() )
      return;
  }
  // insert dir into queue
  if(insertInFrontOfSearchList)
  {
    FileSearchVectorType::iterator it = m_SearchDirectories.begin();
    m_SearchDirectories.insert(it,std::string(dir));
  }
  else
    m_SearchDirectories.push_back(std::string(dir));
}

void mitk::StandardFileLocations::RemoveDirectoryForSearch(const char * dir)
{
  FileSearchVectorType::iterator it;
  // background layers
  if(m_SearchDirectories.size() > 0)
  {
    it = std::find(m_SearchDirectories.begin(), m_SearchDirectories.end(),std::string(dir));
    if(it != m_SearchDirectories.end())
    {
      m_SearchDirectories.erase(it);
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
    AddDirectoryForSearch(mitkConf, false);

  // 2. use .mitk-subdirectory in home directory of the user
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* homeDrive = itksys::SystemTools::GetEnv("HOMEDRIVE");
  const char* homePath = itksys::SystemTools::GetEnv("HOMEPATH");

  if((homeDrive!=NULL) || (homePath!=NULL))
  {
    directoryPath = homeDrive;
    directoryPath += homePath;
    directoryPath += "/.mitk/";
    AddDirectoryForSearch(directoryPath.c_str(), false);
  }

#else
  const char* homeDirectory = itksys::SystemTools::GetEnv("HOME");
  if(homeDirectory != NULL)
  {
    directoryPath = homeDirectory;
    directoryPath += "/.mitk/";
    AddDirectoryForSearch(directoryPath.c_str(), false);
  }

#endif // defined(_WIN32) && !defined(__CYGWIN__)

  // 3. look in the current working directory
  directoryPath = "";
  AddDirectoryForSearch(directoryPath.c_str());

  directoryPath = itksys::SystemTools::GetCurrentWorkingDirectory();
  AddDirectoryForSearch(directoryPath.c_str(), false);

  std::string directoryBinPath = directoryPath + "/bin";
  AddDirectoryForSearch(directoryBinPath.c_str(), false);
  // 4. use a source tree location from compile time
  directoryPath = MITK_ROOT;
  if (pathInSourceDir)
  {
    directoryPath += pathInSourceDir;
  }
  directoryPath += '/';
  AddDirectoryForSearch(directoryPath.c_str(), false);

  return SearchDirectoriesForFile(filename);
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
