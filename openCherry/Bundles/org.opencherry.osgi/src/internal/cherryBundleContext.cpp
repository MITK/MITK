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

#include "cherryBundleContext.h"

#include "cherryBundle.h"
#include "../cherryBundleLoader.h"

namespace cherry {

BundleContext::BundleContext(BundleLoader& loader, IBundle::Pointer bundle,
                  const Poco::Path& persistencyDir) :
   m_BundleLoader(loader),m_Bundle(bundle), m_PersistencyDir(persistencyDir)
{

}

BundleContext::~BundleContext()
{

}

bool BundleContext::operator==(const Object* o) const
{
  if (const IBundleContext* context = dynamic_cast<const IBundleContext*>(o))
    return this->GetThisBundle() == context->GetThisBundle();

  return false;
}

void BundleContext::RegisterService(const std::string& id, Service::Pointer service) const
{
  Platform::GetServiceRegistry().RegisterService(id, service);
}

IBundleContext::Pointer
BundleContext::GetContextForBundle(IBundle::ConstPointer bundle) const
{
  return m_BundleLoader.GetContextForBundle(bundle);
}

BundleEvents&
BundleContext::GetEvents() const
{
  return m_BundleLoader.GetEvents();
}

IBundle::ConstPointer
BundleContext::FindBundle(const std::string& name) const
{
  return m_BundleLoader.FindBundle(name);
}

void
BundleContext::ListBundles(std::vector<IBundle::Pointer>& bundles) const
{
  for (BundleLoader::BundleMap::const_iterator i = m_BundleLoader.m_BundleMap.begin();
      i != m_BundleLoader.m_BundleMap.end(); ++i)
  {
    if (i->second.m_Bundle)
    {
      bundles.push_back(i->second.m_Bundle);
    }
  }
}

Poco::Logger&
BundleContext::GetLogger() const
{
  return m_BundleLoader.GetLogger();
}

Poco::Path
BundleContext::GetPathForLibrary(const std::string& libraryName) const
{
  return m_BundleLoader.GetPathForLibrary(libraryName);
}

Poco::Path
BundleContext::GetPersistentDirectory() const
{
  return m_PersistencyDir;
}

IBundle::Pointer
BundleContext::GetThisBundle() const
{
  return m_Bundle;
}

void
BundleContext::InitPersistency(const std::string& /*persistencyPath*/)
{

}

}
