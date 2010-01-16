/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYBUNDLECONTEXT_H_
#define BERRYBUNDLECONTEXT_H_

#include "../berryIBundleContext.h"
#include "../berryBundleLoader.h"

namespace berry {

class BundleContext : public IBundleContext
{

public:

  berryObjectMacro(BundleContext);

  BundleContext(BundleLoader& loader, SmartPointer<IBundle> bundle,
                  const Poco::Path& persistencyDir);

  ~BundleContext();

  IBundleContext::Pointer GetContextForBundle(SmartPointer<const IBundle> bundle) const;

  BundleEvents& GetEvents() const;

  SmartPointer<const IBundle> FindBundle(const std::string& name) const;

  void ListBundles(std::vector<SmartPointer<IBundle> >& bundles) const;

  Poco::Logger& GetLogger() const;

  // Logger& GetLogger() const;

  Poco::Path GetPathForLibrary(const std::string& libraryName) const;

  Poco::Path GetPersistentDirectory() const;

  void RegisterService(const std::string& id, Service::Pointer service) const;

  SmartPointer<IBundle> GetThisBundle() const;

  void InitPersistency(const std::string& persistencyPath);

  //static std::string GetLoggerName(const Bundle* bundle);

  bool operator==(const Object* o) const;

private:
  BundleLoader& m_BundleLoader;
  SmartPointer<IBundle> m_Bundle;
  const Poco::Path m_PersistencyDir;
};

}  // namespace berry

#endif /*BERRYBUNDLECONTEXT_H_*/
