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

#ifndef BERRYIBUNDLE_H_
#define BERRYIBUNDLE_H_

#include "Poco/Path.h"
#include "Poco/Mutex.h"

#include "berryMacros.h"

#include "berryIBundleManifest.h"

namespace berry {

struct IBundleStorage;
struct IBundleActivator;
struct BundleEvents;

struct BERRY_OSGI IBundle : public Object
{

  berryInterfaceMacro(IBundle, berry);

public:

  enum State { BUNDLE_INSTALLED, BUNDLE_UNINSTALLED, BUNDLE_RESOLVED,
                BUNDLE_STARTING, BUNDLE_ACTIVE, BUNDLE_STOPPING };


  virtual ~IBundle() {};

  virtual IBundleActivator* GetActivator() const = 0;
  virtual const std::string& GetActivatorClass() const = 0;
  virtual const std::string& GetActivatorLibrary() const = 0;

  virtual const std::string& GetCopyright() const = 0;
  virtual const std::string& GetVendor() const = 0;

  virtual IBundleManifest::ActivationPolicy GetActivationPolicy() const = 0;
  virtual bool IsSystemBundle() const = 0;

  virtual std::istream* GetLocalizedResource(const std::string& name) const = 0;
  virtual std::istream* GetResource(const std::string& name) const = 0;

  virtual bool IsActive() const = 0;
  virtual bool IsResolved() const = 0;
  virtual bool IsStarted() const = 0;

  virtual const IBundleManifest& GetManifest() const = 0;
  virtual const std::string& GetName() const = 0;
  virtual const Poco::Path GetPath() const = 0;
  virtual IBundleStorage& GetStorage() = 0;
  // const Version& GetVersion() const;

  virtual const IBundleManifest::Dependencies& GetRequiredBundles() const = 0;

  virtual void Resolve() = 0;

  virtual void Start() = 0;
  virtual void Stop() = 0;

  virtual State GetState() const = 0;
  virtual std::string GetStateString() const = 0;

  virtual BundleEvents& GetEvents() = 0;

  virtual const std::string& GetSymbolicName() const = 0;

};

}  // namespace berry

#endif /*BERRYIBUNDLE_H_*/
