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

#include "berryViewSite.h"

#include "../berryIViewReference.h"
#include "../berryIViewPart.h"
#include "../berryPartPane.h"
#include "../berryImageDescriptor.h"
#include "berryViewDescriptor.h"
#include "berryWorkbenchPage.h"

namespace berry
{

ViewSite::ViewSite(IViewReference::Pointer ref, IViewPart::Pointer view,
    WorkbenchPage* page, const std::string& id,
    const std::string& pluginId, const std::string& registeredName)
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

std::string ViewSite::GetSecondaryId()
{
  return GetPartReference().Cast<IViewReference>()->GetSecondaryId();
}

IViewPart::Pointer ViewSite::GetViewPart()
{
  return GetPart().Cast<IViewPart>();
}

}
