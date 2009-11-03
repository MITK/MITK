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

#include "cherryIntroPartAdapterSite.h"

#include <cherryIWorkbenchPage.h>
#include <cherryISelectionProvider.h>

namespace cherry
{

IntroPartAdapterSite::IntroPartAdapterSite(IWorkbenchPartSite::Pointer site,
    IntroDescriptor::Pointer descriptor) :
  descriptor(descriptor), partSite(site)
{

}

Object::Pointer IntroPartAdapterSite::GetService(const std::string& key)
{
  return partSite->GetService(key);
}

bool IntroPartAdapterSite::HasService(const std::string& key) const
{
  return partSite->HasService(key);
}

std::string IntroPartAdapterSite::GetId() const
{
  return descriptor->GetId();
}

SmartPointer<IWorkbenchPage> IntroPartAdapterSite::GetPage()
{
  return partSite->GetPage();
}

std::string IntroPartAdapterSite::GetPluginId() const
{
  return descriptor->GetPluginId();
}

SmartPointer<ISelectionProvider> IntroPartAdapterSite::GetSelectionProvider()
{
  return partSite->GetSelectionProvider();
}

SmartPointer<IWorkbenchWindow> IntroPartAdapterSite::GetWorkbenchWindow()
{
  return partSite->GetWorkbenchWindow();
}

void IntroPartAdapterSite::SetSelectionProvider(
    SmartPointer<ISelectionProvider> provider)
{
  partSite->SetSelectionProvider(provider);
}

}
