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

#ifndef CHERRYBUNDLELOADER_H_
#define CHERRYBUNDLELOADER_H_

#include "Poco/ClassLoader.h"
#include "Poco/Mutex.h"
#include "Poco/Path.h"
#include "Poco/Any.h"
#include "Poco/SharedPtr.h"
#include "Poco/Logger.h"

#include <typeinfo>
#include <map>

#include "osgi/framework/BundleEvents.h"
#include "osgi/framework/IBundleActivator.h"

#include "internal/cherryBundle.h"
#include "internal/cherrySystemBundle.h"

namespace osgi {

namespace framework {

struct IBundleContext;

}

}

namespace cherry {

class CodeCache;

/**
 * Intentionally no CHERRY_OSGI macro. This class belongs into "internal" but
 * needs to stay here.
 */
class CHERRY_OSGI BundleLoader
{

private:
  typedef Poco::ClassLoader<IBundleActivator> ActivatorClassLoader;
  typedef Poco::SharedPtr<ActivatorClassLoader> ActivatorClassLoaderPtr;

  struct BundleInfo {
    Bundle::Pointer m_Bundle;
    ActivatorClassLoaderPtr m_ClassLoader;
    std::map<std::string, Poco::Any*> m_ClassLoaderMap;
    SmartPointer<IBundleContext> m_Context;
  };

  typedef std::map<std::string, BundleInfo> BundleMap;

  BundleMap m_BundleMap;
  BundleEvents m_BundleEvents;
  CodeCache* m_CodeCache;
  mutable Poco::Logger& m_Logger;

  Bundle::Pointer m_SystemBundle;

  mutable Poco::Mutex m_Mutex;

  bool m_ConsoleLog;

  IBundleActivator* LoadActivator(BundleInfo& bundleInfo);

  friend class InternalPlatform;
  friend class BundleContext;
  void StartSystemBundle(SmartPointer<SystemBundle> bundle);

public:
  BundleLoader(CodeCache* codeCache, Poco::Logger& logger); //, BundleFactory* bundleFactory, BundleContextFactory* bundleContextFactory);
  virtual ~BundleLoader();

  SmartPointer<IBundleContext> GetContextForBundle(IBundle::ConstPointer bundle);
  Bundle::Pointer CreateBundle(const Poco::Path& path);

  BundleEvents& GetEvents();
  IBundle::Pointer FindBundle(const std::string& symbolicName);

  Bundle::Pointer LoadBundle(const Poco::Path& path);
  void LoadBundle(Bundle::Pointer bundle);

  Poco::Path GetPathForLibrary(const std::string& libraryName);
  Poco::Path GetLibraryPathFor(SmartPointer<IBundle> bundle);
  std::string GetContributionsPathFor(SmartPointer<IBundle> bundle);

  Poco::Logger& GetLogger() const;

  void ResolveBundle(SmartPointer<IBundle> bundle);
  void ResolveAllBundles();

  void ReadAllContributions();
  void ReadContributions(SmartPointer<IBundle> bundle);
  void ReadDependentContributions(SmartPointer<IBundle> bundle);

  void ListLibraries(SmartPointer<IBundle> bundle, std::vector<std::string>& list, const std::string& baseDir = "bin/");
  void InstallLibraries(SmartPointer<IBundle> bundle, bool copy = false);

  // start all resolved bundles, except the system bundle
  // (it is assumed, that the system bundle has already
  //  been started)
  void StartAllBundles();
  void StartBundle(SmartPointer<Bundle> bundle);
  void StartDependencies(SmartPointer<Bundle> bundle);

  template<class C>
  C* LoadClass(const std::string& bundleName, const std::string& className);

  template<class C>
  C* LoadClass(const std::string& bundleName, const std::string& className, const std::string& manifestName);


};

}  // namespace cherry

#include "cherryBundleLoader.txx"

#endif /*CHERRYBUNDLELOADER_H_*/
