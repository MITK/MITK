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

#ifndef __CHERRY_BUNDLE_LOADER_TXX__
#define __CHERRY_BUNDLE_LOADER_TXX__

#include <iostream>
#include "cherryLog.h"
#include "osgi/framework/IBundleContext.h"


namespace cherry {

template<class C>
C* BundleLoader::LoadClass(const std::string& bundleName, const std::string& className)
{
  return LoadClass<C>(bundleName, className, C::GetManifestName());
}

template<class C>
C* BundleLoader::LoadClass(const std::string& bundleName, const std::string& className, const std::string& manifestName)
{
  CHERRY_INFO(m_ConsoleLog) << "Trying to load class " << className << " with manifest name " << manifestName << " from bundle " << bundleName;
  BundleInfo& bundleInfo = m_BundleMap[bundleName];

  // check that bundle is started
  if (!bundleInfo.m_Bundle->IsStarted()) this->StartBundle(bundleInfo.m_Bundle);

  Poco::Any* any = bundleInfo.m_ClassLoaderMap[typeid(C).name()];
  if (any == 0)
  {
    CHERRY_INFO(m_ConsoleLog) << "Creating new classloader for type: " << typeid(C).name() << std::endl;
    any = new Poco::Any(Poco::SharedPtr<Poco::ClassLoader<C> >(new Poco::ClassLoader<C>()));
    bundleInfo.m_ClassLoaderMap[typeid(C).name()] = any;
  }

  Poco::SharedPtr<Poco::ClassLoader<C> >& cl =
  Poco::RefAnyCast<Poco::SharedPtr<Poco::ClassLoader<C> > >(*any);

  std::string libName = "lib" + bundleInfo.m_Bundle->GetSymbolicName();
    Poco::Path libPath(bundleInfo.m_Context->GetPathForLibrary(libName));

  if (!cl->isLibraryLoaded(libPath.toString()))
  {
    CHERRY_INFO(m_ConsoleLog) << "Loading library: " << libPath.toString();
    
    try {
    cl->loadLibrary(libPath.toString(), manifestName);
    }
    catch (const Poco::LibraryLoadException& e){
      CHERRY_ERROR << e.displayText();
    }
  }
  else {
    CHERRY_INFO(m_ConsoleLog) << "Library " << libPath.toString() << " already loaded";
  }

  return cl->create(className);
}

}

#endif // __CHERRY_BUNDLE_LOADER_TXX__
