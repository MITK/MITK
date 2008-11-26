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

#ifndef CHERRYBUNDLE_H_
#define CHERRYBUNDLE_H_

#include "Poco/Path.h"
#include "Poco/Mutex.h"

#include "../cherryIBundle.h"
#include "../cherryIBundleStorage.h"

namespace cherry {

class BundleLoader;

class Bundle : public IBundle
{
  
public:
  
  cherryClassMacro(Bundle);
  
  Bundle(BundleLoader& loader, IBundleStorage::Pointer storage);
   ~Bundle();
  
  IBundleActivator* GetActivator() const;
  const std::string& GetActivatorClass() const;
  const std::string& GetActivatorLibrary() const;
  
  const std::string& GetCopyright() const;
  const std::string& GetVendor() const;
  
  IBundleManifest::ActivationPolicy GetActivationPolicy() const;
  
  std::istream* GetLocalizedResource(const std::string& name) const;
  std::istream* GetResource(const std::string& name) const;
  
  bool IsActive() const;
  bool IsResolved() const;
  bool IsStarted() const;
  bool IsSystemBundle() const;
  
  const IBundleManifest& GetManifest() const;
  const std::string& GetName() const;
  const Poco::Path GetPath() const;
  IBundleStorage& GetStorage();
  // const Version& GetVersion() const;
  
  const IBundleManifest::Dependencies& GetRequiredBundles() const;
  
  void Resolve();
  
  void Start();
  void Stop();
  
  State GetState() const;
  std::string GetStateString() const;
  
  BundleEvents& GetEvents();
  
  const std::string& GetSymbolicName() const;
  
  void LoadManifest();
  void SetActivator(IBundleActivator* activator);
  
  bool operator==(const Object* o) const;
  
protected:
  
  IBundleManifest::Pointer m_Manifest;
  IBundleStorage::Pointer m_Storage;
  BundleLoader& m_BundleLoader;
  
  IBundleActivator* m_Activator;
  State m_State;
  
  mutable Poco::Mutex m_Mutex;
};

}  // namespace cherry

#endif /*CHERRYBUNDLE_H_*/
