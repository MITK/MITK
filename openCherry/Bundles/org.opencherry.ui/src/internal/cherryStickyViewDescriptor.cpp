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

#include "cherryStickyViewDescriptor.h"

#include "cherryWorkbenchRegistryConstants.h"

#include <cherryIPageLayout.h>

#include <Poco/String.h>

namespace cherry
{

const std::string StickyViewDescriptor::STICKY_FOLDER_RIGHT =
    "stickyFolderRight";
const std::string StickyViewDescriptor::STICKY_FOLDER_LEFT = "stickyFolderLeft";
const std::string StickyViewDescriptor::STICKY_FOLDER_TOP = "stickyFolderTop";
const std::string StickyViewDescriptor::STICKY_FOLDER_BOTTOM =
    "stickyFolderBottom";

StickyViewDescriptor::StickyViewDescriptor(
    IConfigurationElement::Pointer element) throw (CoreException)
{
  this->configurationElement = element;
  configurationElement->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id);
  if (id.empty())
  {
    //TODO IStatus
//    throw new CoreException(new Status(IStatus.ERROR, element .getNamespace(),
//        0, "Invalid extension (missing id) ", null));//$NON-NLS-1$
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

  std::string location;
  configurationElement->GetAttribute(
      WorkbenchRegistryConstants::ATT_LOCATION, location);
  if (!location.empty())
  {
    if (Poco::icompare(location, "left") == 0)
    {
      direction = IPageLayout::LEFT;
    }
    else if (Poco::icompare(location, "top") == 0)
    {
      direction = IPageLayout::TOP;
    }
    else if (Poco::icompare(location, "bottom") == 0)
    {
      direction = IPageLayout::BOTTOM;
      //no else for right - it is the default value;
    }
  }
  return direction;
}

std::string StickyViewDescriptor::GetId() const
{
  return id;
}

bool StickyViewDescriptor::IsCloseable() const
{
  bool closeable = true;
  std::string closeableString;
  configurationElement->GetAttribute(
      WorkbenchRegistryConstants::ATT_CLOSEABLE, closeableString);
  closeable = closeableString != "false";

  return closeable;
}

bool StickyViewDescriptor::IsMoveable() const
{
  bool moveable = true;
  std::string moveableString;
  configurationElement->GetAttribute(
      WorkbenchRegistryConstants::ATT_MOVEABLE, moveableString);
  moveable = moveableString != "false";

  return moveable;
}

}
