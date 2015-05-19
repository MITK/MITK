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

#include "berryIntroPartAdapterSite.h"

#include <berryIWorkbenchPage.h>
#include <berryISelectionProvider.h>

namespace berry
{

IntroPartAdapterSite::IntroPartAdapterSite(IWorkbenchPartSite::Pointer site,
    IntroDescriptor::Pointer descriptor) :
  descriptor(descriptor), partSite(site)
{

}

Object* IntroPartAdapterSite::GetService(const QString& key)
{
  return partSite->GetService(key);
}

bool IntroPartAdapterSite::HasService(const QString& key) const
{
  return partSite->HasService(key);
}

QString IntroPartAdapterSite::GetId() const
{
  return descriptor->GetId();
}

SmartPointer<IWorkbenchPage> IntroPartAdapterSite::GetPage()
{
  return partSite->GetPage();
}

QString IntroPartAdapterSite::GetPluginId() const
{
  return descriptor->GetPluginId();
}

SmartPointer<ISelectionProvider> IntroPartAdapterSite::GetSelectionProvider()
{
  return partSite->GetSelectionProvider();
}

SmartPointer<Shell> IntroPartAdapterSite::GetShell()
{
  return partSite->GetShell();
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
