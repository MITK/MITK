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

#include "berryBundleManifest.h"

#include "Poco/AutoPtr.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Util/PropertyFileConfiguration.h"

#include <iostream>

namespace berry {

BundleManifest::BundleManifest(std::istream* istr)
: POLICY_EAGER("eager"), POLICY_LAZY("lazy")
{
  this->ParseManifest(istr);
}

BundleManifest::~BundleManifest()
{
  
}

bool BundleManifest::operator==(const Object* o) const
{
  if (const IBundleManifest* manifest = dynamic_cast<const IBundleManifest*>(o))
    return this->GetSymbolicName() == manifest->GetSymbolicName();
  
  return false;
}
  
const std::string& 
BundleManifest::GetActivatorClass() const
{
  return m_Activator;
}
  
const std::string& 
BundleManifest::GetActivatorLibrary() const
{
  return m_ActivatorLibrary;
}
  
const std::string&
BundleManifest::GetCopyright() const
{
  return m_Copyright;
}
 
IBundleManifest::ActivationPolicy 
BundleManifest::GetActivationPolicy() const
{
  return m_ActivationPolicy;
}

bool
BundleManifest::IsSystemBundle() const
{
  return m_SystemBundle;
}

const std::string&
BundleManifest::GetName() const
{
  return m_Name;
}
  
const BundleManifest::Dependencies&
BundleManifest::GetRequiredBundles() const
{
  return m_Dependencies;
}
  
const std::string&
BundleManifest::GetSymbolicName() const
{
  return m_SymbolicName;
}
  
const std::string& 
BundleManifest::GetVendor() const
{
  return m_Vendor;
}
  
//const Version& GetVersion() const;
  
void 
BundleManifest::ParseActivator(const std::string& activator)
{
  Poco::StringTokenizer tokenizer(activator, ";", 
      Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
  
  if (tokenizer.count() == 0)
  {
    m_Activator = "";
    m_ActivatorLibrary = "";
    return;
  }

  m_Activator = tokenizer[0];
  m_Activator = activator;
  
  m_ActivatorLibrary = "";
  if (tokenizer.count() > 1)
  {
    Poco::StringTokenizer tokenizer2(tokenizer[1], "=",
        Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
    
    if (tokenizer2.count() == 2 && tokenizer2[0] == "library")
    {
      m_ActivatorLibrary = tokenizer[1];
    }
  }
  
}
  
void 
BundleManifest::ParseManifest(std::istream* istr)
{
  //BERRY_INFO << "Start parsing manifest\n";
  Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> config(new Poco::Util::PropertyFileConfiguration(*istr));
  
  if (config->hasProperty(BUNDLE_ACTIVATOR))
      this->ParseActivator(config->getString(BUNDLE_ACTIVATOR));
  else
  {
    m_Activator = "";
    m_ActivatorLibrary = "";
  }

    
  m_Copyright = config->getString(BUNDLE_COPYRIGHT, "");
  m_ActivationPolicy = (config->getString(BUNDLE_ACTIVATION_POLICY, POLICY_LAZY) == POLICY_EAGER) ? EAGER : LAZY ;
  m_Name = config->getString(BUNDLE_NAME, "");
  m_SymbolicName = config->getString(BUNDLE_SYMBOLICNAME, "");
  m_Vendor = config->getString(BUNDLE_VENDOR, "");
  m_ManifestVersion = config->getString(MANIFEST_VERSION, "1.0");
  m_SystemBundle = config->getBool(SYSTEM_BUNDLE, false);
  
  if (config->hasProperty(REQUIRE_BUNDLE))
    this->ParseRequiredBundles(config->getString(REQUIRE_BUNDLE));
  
}
  
void 
BundleManifest::ParseRequiredBundles(const std::string& requiredBundles)
{
  Poco::StringTokenizer tokenizer(requiredBundles, ",", 
        Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
  
  Poco::StringTokenizer::Iterator iter;
  for (iter = tokenizer.begin(); iter != tokenizer.end(); iter++)
  {
    Poco::StringTokenizer tokenizer2(*iter, ";", 
       Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
    Dependency dep;
    dep.symbolicName = tokenizer2[0];
    m_Dependencies.push_back(dep);
  }
}

} // namespace berry
