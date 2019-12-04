/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryViewSite.h"

#include "berryIViewReference.h"
#include "berryIViewPart.h"
#include "berryPartPane.h"
#include "berryViewDescriptor.h"
#include "berryWorkbenchPage.h"

namespace berry
{

ViewSite::ViewSite(IViewReference::Pointer ref, IViewPart::Pointer view,
    WorkbenchPage* page, const QString& id,
    const QString& pluginId, const QString& registeredName)
: PartSite(ref, view, page)
{
  SetId(id);
  SetRegisteredName(registeredName);
  SetPluginId(pluginId);
}

ViewSite::ViewSite(IViewReference::Pointer ref, IViewPart::Pointer view,
    WorkbenchPage* page, IViewDescriptor::Pointer desc)
: PartSite(ref, view, page)
{
  SetConfigurationElement(desc.Cast<ViewDescriptor>()->GetConfigurationElement());
}

QString ViewSite::GetSecondaryId()
{
  return GetPartReference().Cast<IViewReference>()->GetSecondaryId();
}

IViewPart::Pointer ViewSite::GetViewPart()
{
  return GetPart().Cast<IViewPart>();
}

}
