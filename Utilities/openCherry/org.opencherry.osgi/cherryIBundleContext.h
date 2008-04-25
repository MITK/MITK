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

#ifndef CHERRYIBUNDLECONTEXT_H_
#define CHERRYIBUNDLECONTEXT_H_

#include "cherryMacros.h"

#include "Poco/Path.h"
#include "Poco/Logger.h"


namespace cherry {

struct BundleEvents;
struct IBundle;

struct CHERRY_OSGI IBundleContext : public Object
{
  
  cherryClassMacro(IBundleContext);
  
  virtual ~IBundleContext() {};
  
  virtual IBundleContext::Pointer GetContextForBundle(SmartPointer<const IBundle> bundle) const = 0;
  
  virtual BundleEvents& GetEvents() const = 0;
  
  virtual SmartPointer<const IBundle> FindBundle(const std::string& name) const = 0;
  
  virtual void ListBundles(std::vector<SmartPointer<IBundle> >& bundles) const = 0;
  
  virtual Poco::Logger& GetLogger() const = 0;
  
  // Logger& GetLogger() const;
  
  virtual Poco::Path GetPathForLibrary(const std::string& libraryName) const = 0;
 
  virtual Poco::Path GetPersistentDirectory() const = 0;
  
  // ServiceRegistry& GetRegistry() const;
  
  virtual SmartPointer<IBundle> GetThisBundle() const = 0;
  
  //static std::string GetLoggerName(const Bundle* bundle);
  
};

}  // namespace cherry

#endif /*CHERRYIBUNDLECONTEXT_H_*/
