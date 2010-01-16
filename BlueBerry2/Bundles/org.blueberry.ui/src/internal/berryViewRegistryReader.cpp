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

#include "berryViewRegistryReader.h"

#include "berryViewRegistry.h"
#include "berryWorkbenchRegistryConstants.h"
#include "berryWorkbenchPlugin.h"

#include "../berryImageDescriptor.h"
#include "../berryPlatformUI.h"

namespace berry
{

std::string ViewRegistryReader::GENERAL_VIEW_ID = "org.blueberry.ui";

ViewRegistryReader::ViewRegistryReader() :
  RegistryReader()
{

}

void ViewRegistryReader::ReadViews(ViewRegistry* out)
{
  // this does not seem to really ever be throwing an the exception
  viewRegistry = out;
  this->ReadRegistry(PlatformUI::PLUGIN_ID,
      WorkbenchRegistryConstants::PL_VIEWS);
}

void ViewRegistryReader::ReadCategory(IConfigurationElement::Pointer element)
{
  try
  {
    Category<IViewDescriptor::Pointer>::Pointer cat(new Category<IViewDescriptor::Pointer>(element));
    viewRegistry->Add(cat);
  }
  catch (CoreException e)
  {
    // log an error since its not safe to show a dialog here
    WorkbenchPlugin::Log(
        "Unable to create view category.", e);//$NON-NLS-1$
  }
}

bool ViewRegistryReader::ReadElement(IConfigurationElement::Pointer element)
{
  std::string elementName = element->GetName();
  if (elementName == WorkbenchRegistryConstants::TAG_VIEW)
  {
    this->ReadView(element);
    return true;
  }
  if (elementName == WorkbenchRegistryConstants::TAG_CATEGORY)
  {
    this->ReadCategory(element);
    this->ReadElementChildren(element);
    return true;
  }
  if (elementName == WorkbenchRegistryConstants::TAG_STICKYVIEW)
  {
    this->ReadSticky(element);
    return true;
  }

  return false;
}

void ViewRegistryReader::ReadSticky(IConfigurationElement::Pointer element)
{
  try
  {
    viewRegistry->Add(StickyViewDescriptor::Pointer(new StickyViewDescriptor(element)));
  }
  catch (CoreException& e)
  {
    //TODO IStatus
    // log an error since its not safe to open a dialog here
//    WorkbenchPlugin.log(
//       "Unable to create sticky view descriptor.", e.getStatus());//$NON-NLS-1$
    WorkbenchPlugin::Log("Unable to create sticky view descriptor.", e);
  }
}

void ViewRegistryReader::ReadView(IConfigurationElement::Pointer element)
{
  try
  {
    ViewDescriptor::Pointer desc(new ViewDescriptor(element));
    viewRegistry->Add(desc);
  }
  catch (CoreException e)
  {
    // log an error since its not safe to open a dialog here
    WorkbenchPlugin::Log(
        "Unable to create view descriptor.", e);//$NON-NLS-1$
  }
}

}
