/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
