/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryViewRegistryReader.h"

#include "berryViewRegistry.h"
#include "berryWorkbenchRegistryConstants.h"
#include "berryWorkbenchPlugin.h"

#include "berryPlatformUI.h"

namespace berry
{

QString ViewRegistryReader::GENERAL_VIEW_ID = "org.blueberry.ui";

ViewRegistryReader::ViewRegistryReader() :
  RegistryReader()
{

}

void ViewRegistryReader::ReadViews(IExtensionRegistry* in, ViewRegistry* out)
{
  // this does not seem to really ever be throwing an the exception
  viewRegistry = out;
  this->ReadRegistry(in, PlatformUI::PLUGIN_ID(),
                     WorkbenchRegistryConstants::PL_VIEWS);
}

void ViewRegistryReader::ReadCategory(const IConfigurationElement::Pointer& element)
{
  try
  {
    Category<IViewDescriptor::Pointer>::Pointer cat(new Category<IViewDescriptor::Pointer>(element));
    viewRegistry->Add(cat);
  }
  catch (const CoreException &e)
  {
    // log an error since its not safe to show a dialog here
    WorkbenchPlugin::Log(
        "Unable to create view category.", e);
  }
}

bool ViewRegistryReader::ReadElement(const SmartPointer<IConfigurationElement> &element)
{
  QString elementName = element->GetName();
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

void ViewRegistryReader::ReadSticky(const SmartPointer<IConfigurationElement> &element)
{
  try
  {
    viewRegistry->Add(StickyViewDescriptor::Pointer(new StickyViewDescriptor(element)));
  }
  catch (const CoreException& e)
  {
    //TODO IStatus
    // log an error since its not safe to open a dialog here
//    WorkbenchPlugin.log(
//       "Unable to create sticky view descriptor.", e.getStatus());//$NON-NLS-1$
    WorkbenchPlugin::Log("Unable to create sticky view descriptor.", e);
  }
}

void ViewRegistryReader::ReadView(const SmartPointer<IConfigurationElement> &element)
{
  try
  {
    ViewDescriptor::Pointer desc(new ViewDescriptor(element));
    viewRegistry->Add(desc);
  }
  catch (const CoreException &e)
  {
    // log an error since its not safe to open a dialog here
    WorkbenchPlugin::Log(
        "Unable to create view descriptor.", e);//$NON-NLS-1$
  }
}

}
