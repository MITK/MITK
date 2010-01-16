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

#ifndef BERRYBUNDLEMANIFEST_H_
#define BERRYBUNDLEMANIFEST_H_

#include "osgi/framework/IBundleManifest.h"


namespace berry {

using namespace osgi::framework;

class BundleManifest : public IBundleManifest
{

public:

  osgiObjectMacro(BundleManifest);

  BundleManifest(std::istream* istr);
  ~BundleManifest();

  const std::string& GetActivatorClass() const;
  const std::string& GetActivatorLibrary() const;
  const std::string& GetCopyright() const;
  ActivationPolicy GetActivationPolicy() const;
  bool IsSystemBundle() const;
  const std::string& GetName() const;
  const IBundleManifest::Dependencies& GetRequiredBundles() const;
  const std::string& GetSymbolicName() const;
  const std::string& GetVendor() const;
  //const Version& GetVersion() const;

  bool operator==(const Object* o) const;

private:

  const std::string POLICY_EAGER;
  const std::string POLICY_LAZY;

  void ParseActivator(const std::string& activator);
  void ParseManifest(std::istream* istr);
  void ParseRequiredBundles(const std::string& requiredBundles);

  IBundleManifest::Dependencies m_Dependencies;
  std::string m_ActivatorLibrary;

  std::string m_Activator;
  std::string m_Copyright;
  ActivationPolicy m_ActivationPolicy;
  std::string m_Name;
  std::string m_SymbolicName;
  std::string m_Vendor;
  std::string m_Version;
  std::string m_ManifestVersion;
  bool m_SystemBundle;
  std::string m_RequireBundle;
};

}  // namespace berry

#endif /*BERRYBUNDLEMANIFEST_H_*/
