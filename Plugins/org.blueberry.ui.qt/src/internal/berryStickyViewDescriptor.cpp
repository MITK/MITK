/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryStickyViewDescriptor.h"

#include "berryWorkbenchRegistryConstants.h"

#include <berryIPageLayout.h>
#include <berryCoreException.h>
#include <berryStatus.h>
#include <berryIContributor.h>

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
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, element->GetContributor()->GetName(),
                                       0, "Invalid extension (missing id)", BERRY_STATUS_LOC));
    throw CoreException(status);
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
