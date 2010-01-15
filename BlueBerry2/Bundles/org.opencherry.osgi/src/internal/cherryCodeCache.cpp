/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cherryLog.h"

#include "cherryCodeCache.h"
#include "cherryInternalPlatform.h"

#include "Poco/Path.h"
#include "Poco/SharedLibrary.h"

#include <fstream>
#include <iostream>
#include <algorithm>

namespace cherry {

CodeCache::CodeCache(const std::string& path) : m_CachePath(path)
{
  CHERRY_INFO(InternalPlatform::GetInstance()->ConsoleLog()) << "Creating CodeCache with path: " << path << std::endl;
  if (!m_CachePath.exists())
  {
    m_CachePath.createDirectory();
  }

}

CodeCache::~CodeCache()
{

}

void
CodeCache::Clear()
{
  CHERRY_INFO(InternalPlatform::GetInstance()->ConsoleLog()) << "Clearing code cache\n";
  std::vector<Poco::File> files;
  m_CachePath.list(files);
  for (std::vector<Poco::File>::iterator iter = files.begin(); iter != files.end(); ++iter)
  {
    iter->remove(true);
  }
}

bool
CodeCache::HasLibrary(const std::string& name)
{
  //CHERRY_INFO << "HasLibrary checks for: " << name;

  std::vector<std::string> files;
  m_CachePath.list(files);

  std::string libName(name);
  //libName.append(Poco::SharedLibrary::suffix());

  std::vector<std::string>::iterator iter;
  for (iter = files.begin(); iter != files.end(); iter++)
  {
    if ((*iter) == libName) {
      //CHERRY_INFO << " FOUND\n";
      return true;
    }
  }

  //CHERRY_INFO << " NOT FOUND\n";
  return false;
}

void
CodeCache::InstallLibrary(const std::string& name, std::istream& istr)
{
  //CHERRY_INFO << "Installing library " << name << " to " << this->GetPathForLibrary(name).toString() << std::endl;
  std::ofstream ostr(this->GetPathForLibrary(name).toString().c_str(), std::ios::binary | std::ios::trunc);

  ostr << istr.rdbuf();
}

void
CodeCache::InstallLibrary(const std::string& name, const Poco::File& path)
{
  //CHERRY_INFO << "Registering library " << name << " in " << path.path() << std::endl;
  m_LibPaths.insert(std::make_pair(name, path));
}

void
CodeCache::UnInstallLibrary(const std::string& name)
{
  std::map<std::string, Poco::File>::iterator iter = m_LibPaths.find(name);

  if (iter == m_LibPaths.end())
  {
    Poco::File(this->GetPathForLibrary(name)).remove();
  }
  else
  {
    m_LibPaths.erase(name);
  }
}

Poco::Path
CodeCache::GetPathForLibrary(const std::string& name)
{
  // We instructed cmake to replace "." with "_" in library names
  // since they are also used for defines (for Windows dll import/export
  // stuff) and . is bad in identifiers.
  // Hence we must replace all "." with "_" here too
  std::string libName(name);
  std::replace(libName.begin(), libName.end(), '.', '_');

  //CHERRY_INFO << "Getting path for library: " << libName << std::endl;
  if (m_LibPaths.find(libName) != m_LibPaths.end())
  {
    return Poco::Path(m_LibPaths[libName].path(), libName + Poco::SharedLibrary::suffix());
  }
  else
  {
    return Poco::Path(m_CachePath.path(), libName + Poco::SharedLibrary::suffix());
  }
}

}
