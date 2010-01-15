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

#ifndef OSGI_FRAMEWORK_IBUNDLESTORAGE_H_
#define OSGI_FRAMEWORK_IBUNDLESTORAGE_H_

#include "Macros.h"
#include "Object.h"

#include "Poco/Path.h"

#include <vector>


namespace osgi {

namespace framework {

struct CHERRY_OSGI IBundleStorage : public Object
{
  osgiInterfaceMacro(osgi::framework::IBundleStorage);

  virtual std::istream* GetResource(const std::string& path) const = 0;
  virtual void List(const std::string& path, std::vector<std::string>& files, bool quiet=true) const = 0;
  virtual bool IsDirectory(const std::string& path) const = 0;
  virtual Poco::Path GetPath() const = 0;

  virtual ~IBundleStorage() {};
};

} } // namespace

#endif /*OSGI_FRAMEWORK_IBUNDLESTORAGE_H_*/
