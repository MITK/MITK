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

namespace cherry {

template<class C>
C* BundleLoader::LoadClass(const std::string& bundleName, const std::string& base, const std::string& className)
{
  m_Logger.information("Trying to load class " + className + " from plugin " + bundleName);
  std::cout << "Trying to load class " << className << " with base " << base << " from bundle " << bundleName << std::endl;
  BundleInfo& bundleInfo = m_BundleMap[bundleName];
  
  // check that bundle is started
  if (!bundleInfo.m_Bundle->IsStarted()) this->StartBundle(bundleInfo.m_Bundle);
  
  Poco::Any* any = bundleInfo.m_ClassLoaderMap[typeid(C).name()]; 
  if (any == 0)
  {
    std::cout << "Creating new classloader for type: " << typeid(C).name() << std::endl;
    any = new Poco::Any(Poco::SharedPtr<Poco::ClassLoader<C> >(new Poco::ClassLoader<C>()));
    bundleInfo.m_ClassLoaderMap[typeid(C).name()] = any;
  }
  
  Poco::SharedPtr<Poco::ClassLoader<C> >& cl = 
    Poco::RefAnyCast<Poco::SharedPtr<Poco::ClassLoader<C> > >(*any);
  
  std::string libName = "lib" + bundleInfo.m_Bundle->GetSymbolicName();
  Poco::Path libPath(bundleInfo.m_Context->GetPathForLibrary(libName));
  
  if (!cl->isLibraryLoaded(libPath.toString()))
  {
    m_Logger.information("Loading library: " + libPath.toString());
    std::cout << "Loading library: " << libPath.toString() << std::endl;
    cl->loadLibrary(libPath.toString(), base);
  }
  else {
    std::cout << "Library " << libPath.toString() << " already loaded\n";
  }
  
  return cl->create(className);
}

}

#endif // __CHERRY_BUNDLE_LOADER_TXX__
