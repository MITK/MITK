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

#ifndef BERRYIBUNDLECONTEXT_H_
#define BERRYIBUNDLECONTEXT_H_

#include "berryMacros.h"

#include "Poco/Path.h"
#include "Poco/Logger.h"

#include "service/berryService.h"

namespace berry {

struct BundleEvents;
struct IBundle;

struct BERRY_OSGI IBundleContext : public Object
{

  berryInterfaceMacro(IBundleContext, berry);

  virtual ~IBundleContext() {};

  virtual IBundleContext::Pointer GetContextForBundle(SmartPointer<const IBundle> bundle) const = 0;

  virtual BundleEvents& GetEvents() const = 0;

  virtual SmartPointer<const IBundle> FindBundle(const std::string& name) const = 0;

  virtual void ListBundles(std::vector<SmartPointer<IBundle> >& bundles) const = 0;

  virtual Poco::Logger& GetLogger() const = 0;

  // Logger& GetLogger() const;

  virtual Poco::Path GetPathForLibrary(const std::string& libraryName) const = 0;

  virtual Poco::Path GetPersistentDirectory() const = 0;

  template<class S>
  SmartPointer<S> GetService(const std::string& id) const;

  virtual void RegisterService(const std::string& id, Service::Pointer service) const = 0;

  virtual SmartPointer<IBundle> GetThisBundle() const = 0;

  //static std::string GetLoggerName(const Bundle* bundle);

};

}  // namespace berry

#include "berryIBundleContext.txx"

#endif /*BERRYIBUNDLECONTEXT_H_*/
