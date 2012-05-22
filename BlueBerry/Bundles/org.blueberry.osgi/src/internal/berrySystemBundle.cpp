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

#include "berrySystemBundle.h"
#include "berrySystemBundleManifest.h"

#include "Poco/Exception.h"

#include "berryBundleLoader.h"

namespace berry {

SystemBundle::SystemBundle(BundleLoader& loader, IBundleStorage::Pointer storage)
 : Bundle(loader, storage, false)
{
  this->init();
  m_State = BUNDLE_RESOLVED;
}

void SystemBundle::Start()
{
  
}

void SystemBundle::Resume()
{
  m_State = BUNDLE_ACTIVE;

  // read the plugin.xml file from the resolved plugins
  try
  {
    m_BundleLoader.ReadAllContributions();
  }
  catch (Poco::Exception exc)
  {
    BERRY_ERROR << exc.displayText() << std::endl;
  }
  
  // start all plugins with lazy-start: false
  try
  {
    m_BundleLoader.StartAllBundles();
  }
  catch (Poco::Exception exc)
  {
    BERRY_ERROR << exc.displayText() << std::endl;
  }

}

BundleLoader& SystemBundle::GetBundleLoader()
{
  return m_BundleLoader;
}

void SystemBundle::LoadManifest()
{
  m_Manifest = new SystemBundleManifest();
}

}
