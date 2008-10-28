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
#include <map>

#include "Poco/File.h"
#include "Poco/Path.h"

namespace cherry {

class CodeCache
{
public:
  CodeCache(const std::string& path);
  virtual ~CodeCache();

  void Clear();

  /**
   * Checks if the library with name name is installed
   *
   * @param name The name of the library without the library suffix
   * @return true if the library is installed, false otherwise
   */
  bool HasLibrary(const std::string& name);

  /**
   * Installs a library in the plugin cache directory under the name name.
   * This is used to copy libraries from Zip files, network locations etc.
   * into the local cache directory.
   *
   * @param name The name of the library without the library suffix
   * @param istr The binary input stream representing the library to be installed
   */
  void InstallLibrary(const std::string& name, std::istream& istr);

  /**
   * Installs the library name. The method only registers a library under the
   * name name in the path path (no files or copied).
   *
   * @param name The name of the library without the library suffix
   * @param path The path to the library
   */
  void InstallLibrary(const std::string& name, const Poco::File& path);

  /**
   * If the library with name name has been copied into the local
   * cache directory, it will be deleted. Otherwise, the library will become
   * unknown to the CodeCache.
   *
   * @param name The name of the library without the library suffix
   */
  void UnInstallLibrary(const std::string& name);

  Poco::Path GetPathForLibrary(const std::string& name);
  //Poco::Path GetPathForFileName(const std::string& name);

private:
  Poco::File m_CachePath;

  std::map<std::string, Poco::File> m_LibPaths;
};

}

#endif /*CHERRYCODECACHE_H_*/
