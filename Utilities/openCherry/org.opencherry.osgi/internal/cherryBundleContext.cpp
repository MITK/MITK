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
#include "cherryBundleLoader.h"

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
    return this->GetThisBundle().CompareTo(context->GetThisBundle());
  
  return false;
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
BundleContext::ListBundles(std::vector<IBundle::Pointer>& /*bundles*/) const
{
  throw Poco::NotImplementedException("BundleContext::ListBundles() not implemented yet");
}
  
Poco::Logger& 
BundleContext::GetLogger() const
{
  
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
