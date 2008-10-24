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

#include "cherryCodeCache.h"

#include "Poco/Path.h"
#include "Poco/SharedLibrary.h"

#include <fstream>
#include <iostream>
#include <algorithm>

namespace cherry {

CodeCache::CodeCache(const std::string& path) : m_CachePath(path)
{
  std::cout << "Creating CodeCache with path: " << path << std::endl;
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
  std::cout << "Clearing code cache\n";
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
  std::vector<std::string> files;
  m_CachePath.list(files);

  std::string libName(name);
  //libName.append(Poco::SharedLibrary::suffix());
  
  std::vector<std::string>::iterator iter;
  for (iter = files.begin(); iter != files.end(); iter++)
  {
    if ((*iter) == libName) return true;
  }
  
  return false;
}
  
void
CodeCache::InstallLibrary(const std::string& name, std::istream& istr)
{
  std::cout << "Installing library " << name << " to " << this->GetPathForFileName(name).toString() << std::endl;
  std::ofstream ostr(this->GetPathForFileName(name).toString().c_str(), std::ios::binary | std::ios::trunc);
  
  ostr << istr.rdbuf();
}

void
CodeCache::UnInstallLibrary(const std::string& name)
{
  Poco::File(this->GetPathForFileName(name)).remove();
}
  
Poco::Path
CodeCache::GetPathForLibName(const std::string& name)
{
  // We instructed cmake to replace "." with "_" in library names
  // since they are also used for defines (for Windows dll import/export
  // stuff) and . is bad in identifiers.
  // Hence we must replace all "." with "_" here too
  std::string libName(name);
  std::replace(libName.begin(), libName.end(), '.', '_');
  libName += Poco::SharedLibrary::suffix();
  return Poco::Path(m_CachePath.path(), libName).toString();
}

Poco::Path
CodeCache::GetPathForFileName(const std::string& name)
{
  return Poco::Path(m_CachePath.path(), name).toString();
}

}
