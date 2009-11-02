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

#include "cherryViewIntroAdapterSite.h"

#include <cherryIWorkbenchPage.h>
#include <cherryISelectionProvider.h>

namespace cherry
{

ViewIntroAdapterSite::ViewIntroAdapterSite(IViewSite::Pointer viewSite,
    IntroDescriptor::Pointer descriptor) :
  descriptor(descriptor), viewSite(viewSite)
{

}

Object::Pointer ViewIntroAdapterSite::GetService(const std::string& key)
{
  return viewSite->GetService(key);
}

bool ViewIntroAdapterSite::HasService(const std::string& key) const
{
  return viewSite->HasService(key);
}

std::string ViewIntroAdapterSite::GetId() const
{
  return descriptor->GetId();
}

SmartPointer<IWorkbenchPage> ViewIntroAdapterSite::GetPage()
{
  return viewSite->GetPage();
}

std::string ViewIntroAdapterSite::GetPluginId() const
{
  return descriptor->GetPluginId();
}

SmartPointer<ISelectionProvider> ViewIntroAdapterSite::GetSelectionProvider()
{
  return viewSite->GetSelectionProvider();
}

SmartPointer<IWorkbenchWindow> ViewIntroAdapterSite::GetWorkbenchWindow()
{
  return viewSite->GetWorkbenchWindow();
}

void ViewIntroAdapterSite::SetSelectionProvider(
    SmartPointer<ISelectionProvider> provider)
{
  viewSite->SetSelectionProvider(provider);
}

}
