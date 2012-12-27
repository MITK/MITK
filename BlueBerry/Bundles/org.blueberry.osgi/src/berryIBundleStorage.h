/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYIBUNDLESTORAGE_H_
#define BERRYIBUNDLESTORAGE_H_

#include "berryMacros.h"
#include "berryObject.h"

#include "Poco/Path.h"

#include <vector>


namespace berry {

struct BERRY_OSGI IBundleStorage : public Object
{
  berryInterfaceMacro(IBundleStorage, berry);

  virtual std::istream* GetResource(const std::string& path) const = 0;
  virtual void List(const std::string& path, std::vector<std::string>& files, bool quiet=true) const = 0;
  virtual bool IsDirectory(const std::string& path) const = 0;
  virtual Poco::Path GetPath() const = 0;

  virtual ~IBundleStorage() {};
};

} // namespace berry

#endif /*BERRYIBUNDLESTORAGE_H_*/
