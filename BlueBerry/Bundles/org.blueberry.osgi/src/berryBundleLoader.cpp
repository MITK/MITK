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


#include "berryLog.h"

#include "berryBundleLoader.h"

#include "internal/berryBundleContext.h"
#include "internal/berryBundleDirectory.h"
#include "event/berryBundleEvents.h"

#include "internal/berryDefaultActivator.h"
#include "internal/berrySystemBundleActivator.h"
#include "internal/berryCodeCache.h"

#include "internal/berryInternalPlatform.h"

#include "berryPlugin.h"
#include "berryPlatform.h"
#include "berryPlatformException.h"

#include "service/berryIExtensionPointService.h"

#include <Poco/Environment.h>

namespace berry {


BundleLoader::BundleLoader(CodeCache* codeCache, Poco::Logger& logger) //, BundleFactory* bundleFactory, BundleContextFactory* bundleContextFactory);
 : m_CodeCache(codeCache), m_Logger(logger), m_ConsoleLog(false)
{
  m_ConsoleLog = InternalPlatform::GetInstance()->ConsoleLog();
}

BundleLoader::~BundleLoader()
{

}

void BundleLoader::SetCTKPlugins(const QStringList& installedCTKPlugins)
{
  this->installedCTKPlugins = installedCTKPlugins;
}

Poco::Logger&
BundleLoader::GetLogger() const
{
  return m_Logger;
}

IBundleContext::Pointer
BundleLoader::GetContextForBundle(IBundle::ConstPointer bundle)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  return m_BundleMap[bundle->GetSymbolicName()].m_Context;
}

Bundle::Pointer
BundleLoader::CreateBundle(const Poco::Path& path)
{
  BundleDirectory::Pointer bundleStorage(new BundleDirectory(path));
  Bundle::Pointer bundle(new Bundle(*this, bundleStorage));

  if (bundle->GetState() == IBundle::BUNDLE_INSTALLED &&
      installedCTKPlugins.contains(QString::fromStdString(bundle->GetSymbolicName())))
  {
    BERRY_WARN << "Ignoring legacy BlueBerry bundle " << bundle->GetSymbolicName()
               << " because a CTK plug-in with the same name already exists.";
    return Bundle::Pointer(0);
  }

  if (bundle->GetState() == IBundle::BUNDLE_INSTALLED &&
      bundle->IsSystemBundle()) {
    bundle = new SystemBundle(*this, bundleStorage);
    m_SystemBundle = bundle;
  }

  //BundleEvent event(bundle, BundleEvent::EV_BUNDLE_INSTALLED);
  //m_BundleEvents.bundleInstalled(this, event);

  return bundle;
}

BundleEvents&
BundleLoader::GetEvents()
{
  return m_BundleEvents;
}

IBundle::Pointer
BundleLoader::FindBundle(const std::string& symbolicName)
{
  if (symbolicName == "system.bundle") return m_SystemBundle;

  Poco::Mutex::ScopedLock lock(m_Mutex);
  BundleMap::const_iterator iter;
  iter = m_BundleMap.find(symbolicName);
  if (iter == m_BundleMap.end()) return IBundle::Pointer();
  return iter->second.m_Bundle;
}

std::vector<IBundle::Pointer> BundleLoader::GetBundles() const
{
  std::vector<IBundle::Pointer> result;
  BundleMap::const_iterator end = m_BundleMap.end();
  for (BundleMap::const_iterator it = m_BundleMap.begin(); it != end; ++it)
  {
    result.push_back(it->second.m_Bundle);
  }
  return result;
}

Bundle::Pointer
BundleLoader::LoadBundle(const Poco::Path& path)
{
  Bundle::Pointer bundle = this->CreateBundle(path);
  if (bundle) this->LoadBundle(bundle);
  return bundle;
}

void
BundleLoader::LoadBundle(Bundle::Pointer bundle)
{
  if (bundle->GetState() == IBundle::BUNDLE_INSTALLED ||
      bundle->GetState() == IBundle::BUNDLE_RESOLVED)
  {
    Poco::Mutex::ScopedLock lock(m_Mutex);
    if (m_BundleMap[bundle->GetSymbolicName()].m_Bundle.IsNull())
    {
      BundleInfo bundleInfo;
      bundleInfo.m_Bundle = bundle;
      bundleInfo.m_ClassLoader = new ActivatorClassLoader();
      Poco::Path path; Platform::GetStatePath(path, bundle);
      bundleInfo.m_Context = new BundleContext(*this, bundle, path);
      m_BundleMap[bundle->GetSymbolicName()] = bundleInfo;

      this->InstallLibraries(bundle);

      //BundleEvent event(bundle, BundleEvent::EV_BUNDLE_LOADED);
      //m_BundleEvents.bundleLoaded(this, event);
    }
    else
    {
      //TODO version conflict check
    }
  }
  else
  {
    throw BundleStateException("The bundle must be in state INSTALLED in order to be loaded.");
  }
}

Poco::Path
BundleLoader::GetPathForLibrary(const std::string& libraryName)
{
  return m_CodeCache->GetPathForLibrary(libraryName);
}

Poco::Path
BundleLoader::GetLibraryPathFor(IBundle::Pointer bundle)
{
  std::string libName = bundle->GetActivatorLibrary();
  if (libName.empty()) libName = "lib" + bundle->GetSymbolicName();
  return this->GetPathForLibrary(libName);
}

std::string
BundleLoader::GetContributionsPathFor(IBundle::Pointer /*bundle*/)
{
  return "plugin.xml";
}

void
BundleLoader::ResolveBundle(IBundle::Pointer bundle)
{
  try
  {
    BERRY_INFO(m_ConsoleLog) << "Trying to resolve bundle " << bundle->GetSymbolicName();
    bundle->Resolve();
    BERRY_INFO(m_ConsoleLog) << "Bundle " << bundle->GetSymbolicName() << ": " << bundle->GetStateString();
  }
  catch (BundleResolveException exc)
  {
    BERRY_ERROR << "Bundle resolve failed: " << exc.displayText();
  }

//  if (bundle->IsResolved())
//  {
//    BundleEvent event(bundle, BundleEvent::EV_BUNDLE_RESOLVED);
//    m_BundleEvents.bundleResolved(this, event);
//  }

}

void
BundleLoader::ResolveAllBundles()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  BundleMap::iterator iter;
  for (iter = m_BundleMap.begin(); iter != m_BundleMap.end(); iter++)
  {
    this->ResolveBundle(iter->second.m_Bundle);
  }
}

void
BundleLoader::ListLibraries(IBundle::Pointer bundle, std::vector<std::string>& list)
{
  ListLibraries(bundle, list, "bin/");
#ifdef CMAKE_INTDIR
  ListLibraries(bundle, list, "bin/" CMAKE_INTDIR "/");
#endif
}

void
BundleLoader::ListLibraries(IBundle::Pointer bundle, std::vector<std::string>& list, const std::string& baseDir)
{
  std::vector<std::string> tmpList;
  bundle->GetStorage().List(baseDir, tmpList);

  std::string::size_type suf = Poco::SharedLibrary::suffix().size();
  std::vector<std::string>::iterator iter;
  for (iter = tmpList.begin(); iter != tmpList.end(); )
  {
    if (bundle->GetStorage().IsDirectory(baseDir + *iter))
    {
      // uncomment the following line for a recursive lookup
      //this->ListLibraries(bundle, list, baseDir + *iter + "/");
      iter = tmpList.erase(iter);
    }
    else
    {
      if (iter->substr(iter->size() - suf) == Poco::SharedLibrary::suffix())
      {
        iter->erase(iter->size() - suf);
        iter->insert(0, baseDir);
        ++iter;
      }
      else
      {
        iter = tmpList.erase(iter);
      }

    }
  }

  list.insert(list.end(), tmpList.begin(), tmpList.end());
}

void
BundleLoader::InstallLibraries(IBundle::Pointer bundle, bool copy)
{
  std::vector<std::string> libraries;
  this->ListLibraries(bundle, libraries);

  std::vector<std::string>::iterator iter;
  for (iter = libraries.begin(); iter != libraries.end(); ++iter)
  {
    if (iter->empty()) continue;

    //BERRY_INFO << "Testing CodeCache for: " << *iter << std::endl;

    std::size_t separator = iter->find_last_of("/");
    std::string libFileName = *iter;
    if (separator != std::string::npos)
      libFileName = iter->substr(separator+1);

    if (!m_CodeCache->HasLibrary(libFileName))
    {
      std::string libDir = "";
      if (separator != std::string::npos)
          libDir += iter->substr(0, separator);

      // Check if we should copy the dll (from a ZIP file for example)
      if (copy)
      {
        //TODO This copies all files which start with *iter to the
        // plugin cache. This is necessary for Windows, for example,
        // where a .dll file is accompanied by a set of other files.
        // This should be extended to check for the right dll files, since
        // it would be possible (and a good idea anyway) to put multiple
        // versions of the same library in the ZIP file, targeting different
        // compilers for example.
        std::vector<std::string> files;
        bundle->GetStorage().List(libDir, files);
        for (std::vector<std::string>::iterator fileName = files.begin();
             fileName != files.end(); ++fileName)
        {
          std::size_t size = std::min<std::size_t>(libFileName.size(), fileName->size());
          if (fileName->compare(0, size, libFileName) != 0) continue;

          std::istream* istr = bundle->GetResource(libDir + *fileName);
          m_CodeCache->InstallLibrary(*iter, *istr);
          delete istr;
        }
      }
      else
      {
        Poco::Path bundlePath = bundle->GetStorage().GetPath();
        bundlePath.append(Poco::Path::forDirectory(libDir));

        // On Windows, we set the path environment variable to include
        // the path to the library, so the loader can find it. We do this
        // programmatically because otherwise, a user would have to edit
        // a batch file every time he adds a new plugin from outside the
        // build system.
        #ifdef BERRY_OS_FAMILY_WINDOWS
        std::string pathEnv = Poco::Environment::get("PATH", "");
        if (!pathEnv.empty()) pathEnv += ";";
        pathEnv += bundlePath.toString();
        Poco::Environment::set("PATH", pathEnv);
        #endif

        m_CodeCache->InstallLibrary(libFileName, bundlePath);
      }
    }
  }
}

void BundleLoader::ReadAllContributions()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  BundleMap::iterator iter;
  for (iter = m_BundleMap.begin(); iter != m_BundleMap.end(); ++iter)
  {
    if (iter->second.m_Bundle && iter->second.m_Bundle->IsResolved())
      this->ReadContributions(iter->second.m_Bundle);
  }
}

void BundleLoader::ReadContributions(IBundle::Pointer bundle)
{
  this->ReadDependentContributions(bundle);

  IExtensionPointService::Pointer service = Platform::GetExtensionPointService();
  if (service->HasContributionFrom(bundle->GetSymbolicName())) return;

  std::istream* istr = bundle->GetResource(this->GetContributionsPathFor(bundle));
  if (istr)
  {
    service->AddContribution(*istr, bundle->GetSymbolicName());
    delete istr;
  }
}

void BundleLoader::ReadDependentContributions(IBundle::Pointer bundle)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  const IBundleManifest::Dependencies& deps = bundle->GetRequiredBundles();

  IBundleManifest::Dependencies::const_iterator iter;
  for (iter = deps.begin(); iter != deps.end(); ++iter)
  {
    BundleMap::const_iterator depIter = m_BundleMap.find(iter->symbolicName);
    if (depIter != m_BundleMap.end())
    {
      // only read contributions from legacy BlueBerry bundles
      if (IBundle::Pointer depBundle = depIter->second.m_Bundle)
      {
        this->ReadContributions(depBundle);
      }
    }
  }

}

void
BundleLoader::StartAllBundles()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  BundleMap::iterator iter;
  for (iter = m_BundleMap.begin(); iter != m_BundleMap.end(); ++iter)
  {
    try
    {
      if  (iter->second.m_Bundle && iter->second.m_Bundle->GetActivationPolicy() == IBundleManifest::EAGER  &&
        !iter->second.m_Bundle->IsSystemBundle())
        this->StartBundle(iter->second.m_Bundle);
    }
    catch (Poco::Exception exc)
    {
      BERRY_ERROR << exc.displayText() << std::endl;
    }
  }
}

void
BundleLoader::StartBundle(Bundle::Pointer bundle)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);

  if (bundle->GetState() != IBundle::BUNDLE_RESOLVED) return;

  this->StartDependencies(bundle);

  BundleInfo info = m_BundleMap[bundle->GetSymbolicName()];
  IBundleActivator* activator = this->LoadActivator(info);

  Plugin* plugin = dynamic_cast<Plugin*>(activator);
  if (plugin) plugin->m_Bundle = bundle;

  bundle->SetActivator(activator);

  bundle->Start();
}

void
BundleLoader::StartSystemBundle(SystemBundle::Pointer bundle)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);

  if (bundle->IsStarted()) return;

  BundleInfo info = m_BundleMap[bundle->GetSymbolicName()];
  //IBundleActivator* activator = this->LoadActivator(info);
  IBundleActivator* activator = new SystemBundleActivator();

  bundle->SetActivator(activator);

  activator->Start(info.m_Context);
}

void
BundleLoader::StartDependencies(Bundle::Pointer bundle)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);

  const IBundleManifest::Dependencies& deps = bundle->GetRequiredBundles();

  IBundleManifest::Dependencies::const_iterator iter;
  QList<QSharedPointer<ctkPlugin> > ctkPlugins = CTKPluginActivator::getPluginContext()->getPlugins();
  for (iter = deps.begin(); iter != deps.end(); ++iter)
  {
    BundleMap::const_iterator depIter = m_BundleMap.find(iter->symbolicName);
    if (depIter != m_BundleMap.end())
    {
      if (Bundle::Pointer depBundle = depIter->second.m_Bundle)
      {
        this->StartBundle(depBundle);
      }
    }
    else
    {
      foreach (QSharedPointer<ctkPlugin> ctkPlugin, ctkPlugins)
      {
        if (ctkPlugin->getSymbolicName() == QString::fromStdString(iter->symbolicName))
        {
          ctkPlugin->start(0);
          break;
        }
      }
    }
  }
}

IBundleActivator*
BundleLoader::LoadActivator(BundleInfo& bundleInfo)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  std::string activator = bundleInfo.m_Bundle->GetActivatorClass();

  if (activator == "") return new DefaultActivator();

  Poco::Path libPath = this->GetLibraryPathFor(bundleInfo.m_Bundle);
  std::string strLibPath(libPath.toString());
  
  BERRY_INFO(m_ConsoleLog) << "Loading activator library: " << strLibPath;
  try
  {
  /* retrieves only an empty string and its not required 
#ifdef BERRY_OS_FAMILY_WINDOWS
    char cDllPath[512];
    GetDllDirectory(512, cDllPath);
    BERRY_INFO << "Dll Path: " << cDllPath << std::endl;
#endif
  */
    bundleInfo.m_ClassLoader->loadLibrary(strLibPath);
    return bundleInfo.m_ClassLoader->create(activator);
  }
  catch (Poco::LibraryLoadException exc)
  {
    BERRY_ERROR << "Could not create Plugin activator. Did you export the class \"" << activator << "\" ?\n"
                 << "  Exception displayText(): " << exc.displayText();
    exc.rethrow();
  }

  return 0;
}

} // namespace berry
