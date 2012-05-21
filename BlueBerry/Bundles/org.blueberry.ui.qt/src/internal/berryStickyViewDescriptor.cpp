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

#include "berryStickyViewDescriptor.h"

#include "berryWorkbenchRegistryConstants.h"

#include <berryIPageLayout.h>
#include <berryPlatformException.h>

namespace berry
{

const QString StickyViewDescriptor::STICKY_FOLDER_RIGHT =
    "stickyFolderRight";
const QString StickyViewDescriptor::STICKY_FOLDER_LEFT = "stickyFolderLeft";
const QString StickyViewDescriptor::STICKY_FOLDER_TOP = "stickyFolderTop";
const QString StickyViewDescriptor::STICKY_FOLDER_BOTTOM =
    "stickyFolderBottom";

StickyViewDescriptor::StickyViewDescriptor(IConfigurationElement::Pointer element)
{
  this->configurationElement = element;
  this->id = configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
  if (id.isEmpty())
  {
    //TODO IStatus
//    throw new CoreException(new Status(IStatus.ERROR, element .getNamespace(),
//        0, "Invalid extension (missing id) ", null));
    throw CoreException(element->GetContributor() + ": Invalid extension (missing id)");
  }
}

IConfigurationElement::Pointer StickyViewDescriptor::GetConfigurationElement() const
{
  return configurationElement;
}

int StickyViewDescriptor::GetLocation() const
{
  int direction = IPageLayout::RIGHT;

  QString location = configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_LOCATION);
  if (!location.isEmpty())
  {
    if (location.compare("left", Qt::CaseInsensitive) == 0)
    {
      direction = IPageLayout::LEFT;
    }
    else if (location.compare("top", Qt::CaseInsensitive) == 0)
    {
      direction = IPageLayout::TOP;
    }
    else if (location.compare("bottom", Qt::CaseInsensitive) == 0)
    {
      direction = IPageLayout::BOTTOM;
      //no else for right - it is the default value;
    }
  }
  return direction;
}

QString StickyViewDescriptor::GetId() const
{
  return id;
}

bool StickyViewDescriptor::IsCloseable() const
{
  bool closeable = true;
  QString closeableString = configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_CLOSEABLE);
  closeable = (closeableString != "false");

  return closeable;
}

bool StickyViewDescriptor::IsMoveable() const
{
  bool moveable = true;
  QString moveableString = configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_MOVEABLE);
  moveable = moveableString != "false";

  return moveable;
}

}
