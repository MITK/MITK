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

#ifndef CHERRYCODECACHE_H_
#define CHERRYCODECACHE_H_

#include <string>
#include <iostream>

#include "Poco/File.h"
#include "Poco/Path.h"

namespace cherry {

class CodeCache
{
public:
  CodeCache(const std::string& path);
  virtual ~CodeCache();
  
  void Clear();
  bool HasLibrary(const std::string& name);
  void InstallLibrary(const std::string& name, std::istream& istr);
  void UnInstallLibrary(const std::string& name);
  
  Poco::Path GetPathFor(const std::string& name);
  
private:
  Poco::File m_CachePath;
};

}

#endif /*CHERRYCODECACHE_H_*/
