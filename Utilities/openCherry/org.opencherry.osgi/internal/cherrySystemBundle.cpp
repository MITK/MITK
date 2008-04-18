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

#include "cherrySystemBundle.h"

#include "Poco/Exception.h"

#include "../cherryBundleLoader.h"

namespace cherry {

SystemBundle::SystemBundle(BundleLoader& loader, IBundleStorage::Pointer storage)
 : Bundle(loader, storage)
{
  m_State = BUNDLE_RESOLVED;
}

void SystemBundle::Start()
{
  
}

void SystemBundle::Resume()
{
  // read the plugin.xml file from the resolved plugins
  try
  {
    m_BundleLoader.ReadAllContributions();
  }
  catch (Poco::Exception exc)
  {
    std::cout << exc.displayText() << std::endl;
  }
  
  // start all plugins with lazy-start: false
  try
  {
    m_BundleLoader.StartAllBundles();
  }
  catch (Poco::Exception exc)
  {
    std::cout << exc.displayText() << std::endl;
  }

}

BundleLoader& SystemBundle::GetBundleLoader()
{
  return m_BundleLoader;
}

}
