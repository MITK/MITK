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

#include "cherryBundleLoader.h"

#include "internal/cherryBundleContext.h"
#include "internal/cherryBundleDirectory.h"
#include "event/cherryBundleEvents.h"

#include "internal/cherryDefaultActivator.h"
#include "internal/cherrySystemBundleActivator.h"
#include "internal/cherryCodeCache.h"

#include "cherryPlugin.h"
#include "cherryPlatform.h"
#include "cherryPlatformException.h"

#include "service/cherryIExtensionPointService.h"


namespace cherry {


BundleLoader::BundleLoader(CodeCache* codeCache, Poco::Logger& logger) //, BundleFactory* bundleFactory, BundleContextFactory* bundleContextFactory);
 : m_CodeCache(codeCache), m_Logger(logger)
{

}

BundleLoader::~BundleLoader()
{

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

Bundle::Pointer
BundleLoader::LoadBundle(const Poco::Path& path)
{
  Bundle::Pointer bundle = this->CreateBundle(path);
  this->LoadBundle(bundle);
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
      bundleInfo.m_Context = new BundleContext(*this, bundle, Platform::GetStatePath(bundle));
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
BundleLoader::GetLibraryPathFor(IBundle* bundle)
{
  std::string libName = bundle->GetActivatorLibrary();
  if (libName.empty()) libName = "lib" + bundle->GetSymbolicName();
  return this->GetPathForLibrary(libName);
}

std::string
BundleLoader::GetContributionsPathFor(IBundle* /*bundle*/)
{
  return "plugin.xml";
}

void
BundleLoader::ResolveBundle(IBundle* bundle)
{
  try
  {
    std::cout << "\nTrying to resolve bundle " << bundle->GetSymbolicName() << std::endl;
    bundle->Resolve();
    m_Logger.information("Bundle " + bundle->GetSymbolicName() + " resolved");
    std::cout << "Bundle " << bundle->GetSymbolicName() << ": " << bundle->GetStateString() << "\n\n";
  }
  catch (BundleResolveException exc)
  {
    m_Logger.log(exc);
    std::cout << exc.displayText() << std::endl;
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
BundleLoader::ListLibraries(IBundle* bundle, std::vector<std::string>& list)
{
  std::string libDir = "bin/";
  std::vector<std::string> tmpList;
  bundle->GetStorage().List(libDir, tmpList);

  int suf = Poco::SharedLibrary::suffix().size();
  std::vector<std::string>::iterator iter;
  for (iter = tmpList.begin(); iter != tmpList.end(); )
  {
    if (bundle->GetStorage().IsDirectory("bin/" + *iter))
    {
      std::cout << "Found directory: " << ("bin/" + *iter) << std::endl;
      this->ListLibraries(bundle, list);
    }
    else
    {
      if (iter->substr(iter->size() - suf) == Poco::SharedLibrary::suffix())
      {
        iter->erase(iter->size() - suf);
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
BundleLoader::InstallLibraries(IBundle* bundle, bool copy)
{
  std::vector<std::string> libraries;
  this->ListLibraries(bundle, libraries);

  std::vector<std::string>::iterator iter;
  for (iter = libraries.begin(); iter != libraries.end(); ++iter)
  {
    if (iter->empty()) continue;

    std::cout << "Testing CodeCache for: " << *iter << std::endl;

    std::size_t separator = iter->find_last_of("/");
    std::string libFileName = *iter;
    if (separator != std::string::npos)
      libFileName = iter->substr(separator+1);

    if (!m_CodeCache->HasLibrary(libFileName))
    {
      std::string libDir = "bin/";
      if (separator != std::string::npos)
          libDir += libFileName.substr(0, separator);

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
          int size = std::min<int>(libFileName.size(), fileName->size());
          if (fileName->compare(0, size, libFileName) != 0) continue;

          std::istream* istr = bundle->GetResource(libDir + *fileName);
          m_CodeCache->InstallLibrary(*iter, *istr);
          delete istr;
        }
      }
      else
      {
        Poco::Path bundlePath = bundle->GetStorage().GetPath();
        bundlePath.append(libDir);

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
    if (iter->second.m_Bundle->IsResolved())
      this->ReadContributions(iter->second.m_Bundle);
  }
}

void BundleLoader::ReadContributions(IBundle* bundle)
{
  this->ReadDependentContributions(bundle);

  IExtensionPointService* service = Platform::GetExtensionPointService();
  if (service->HasContributionFrom(bundle->GetSymbolicName())) return;

  try
  {
  std::istream* istr = bundle->GetResource(this->GetContributionsPathFor(bundle));
  service->AddContribution(*istr, bundle->GetSymbolicName());
  delete istr;
  }
  catch (Poco::FileNotFoundException exc)
  {
    //std::cout << exc.displayText() << std::endl;
  }
}

void BundleLoader::ReadDependentContributions(IBundle* bundle)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  const IBundleManifest::Dependencies& deps = bundle->GetRequiredBundles();

  IBundleManifest::Dependencies::const_iterator iter;
  for (iter = deps.begin(); iter != deps.end(); ++iter)
  {
    this->ReadContributions(m_BundleMap[iter->symbolicName].m_Bundle);
  }

}

void
BundleLoader::StartAllBundles()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  BundleMap::iterator iter;
  for (iter = m_BundleMap.begin(); iter != m_BundleMap.end(); ++iter)
  {
    if (!iter->second.m_Bundle->GetLazyStart() &&
        !iter->second.m_Bundle->IsSystemBundle())
      this->StartBundle(iter->second.m_Bundle);
  }
}

void
BundleLoader::StartBundle(Bundle* bundle)
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
BundleLoader::StartSystemBundle(SystemBundle* bundle)
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
BundleLoader::StartDependencies(Bundle* bundle)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);

  const IBundleManifest::Dependencies& deps = bundle->GetRequiredBundles();

  IBundleManifest::Dependencies::const_iterator iter;
  for (iter = deps.begin(); iter != deps.end(); ++iter)
  {
    this->StartBundle(m_BundleMap[iter->symbolicName].m_Bundle);
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
  m_Logger.information("Loading activator library: " + strLibPath);
  std::cout << "Loading activator library: " << strLibPath << std::endl;
  try
  {
    bundleInfo.m_ClassLoader->loadLibrary(strLibPath);
    return bundleInfo.m_ClassLoader->create(activator);
  }
  catch (Poco::LibraryLoadException exc)
  {
    std::cout << "Could not create Plugin activator. Did you export the class \"" << activator << "\" ?\n";
    exc.rethrow();
  }

  return 0;
}

} // namespace cherry
