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

#ifndef CHERRYBUNDLECONTEXT_H_
#define CHERRYBUNDLECONTEXT_H_

#include "cherryIBundleContext.h"
#include "cherryBundleLoader.h"

namespace cherry {

class BundleContext : public IBundleContext
{
  
public:
  
  cherryClassMacro(BundleContext);
  
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
  
  // ServiceRegistry& GetRegistry() const;
  
  SmartPointer<IBundle> GetThisBundle() const;
  
  void InitPersistency(const std::string& persistencyPath);
  
  //static std::string GetLoggerName(const Bundle* bundle);
  
  bool operator==(const Object* o) const;
  
private:
  BundleLoader& m_BundleLoader;
  SmartPointer<IBundle> m_Bundle;
  const Poco::Path m_PersistencyDir;
};

}  // namespace cherry

#endif /*CHERRYBUNDLECONTEXT_H_*/
