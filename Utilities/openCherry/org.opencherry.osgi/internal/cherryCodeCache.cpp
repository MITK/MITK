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
  libName.append(Poco::SharedLibrary::suffix());
  
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
  std::cout << "Installing library " << name << " to " << this->GetPathFor(name).toString() << std::endl;
  std::ofstream ostr(this->GetPathFor(name).toString().c_str(), std::ios::binary | std::ios::trunc);
  
  ostr << istr.rdbuf();
}

void
CodeCache::UnInstallLibrary(const std::string& name)
{
  Poco::File(this->GetPathFor(name)).remove();
}
  
Poco::Path
CodeCache::GetPathFor(const std::string& name)
{
  return Poco::Path(m_CachePath.path(), name + Poco::SharedLibrary::suffix()).toString();
}

}
