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

#include "cherryViewDescriptor.h"

#include "service/cherryIConfigurationElement.h"
#include "cherryPlatformException.h"

#include "cherryRegistryReader.h"
#include "cherryWorkbenchRegistryConstants.h"

#include <Poco/String.h>
#include <Poco/StringTokenizer.h>

namespace cherry
{

ViewDescriptor::ViewDescriptor(IConfigurationElement::Pointer e)
 : configElement(e)
{
  this->LoadFromExtension();
}

IViewPart::Pointer ViewDescriptor::CreateView()
{
  return configElement->CreateExecutableExtension<IViewPart>(
      WorkbenchRegistryConstants::ATT_CLASS);
}

const std::vector<std::string>& ViewDescriptor::GetCategoryPath()
{
  return categoryPath;
}

IConfigurationElement::Pointer ViewDescriptor::GetConfigurationElement()
{
  return configElement;
}

std::string ViewDescriptor::GetDescription()
{
  return RegistryReader::GetDescription(configElement);
}

std::string ViewDescriptor::GetId() const
{
  return id;
}

bool ViewDescriptor::operator==(const IViewDescriptor* o) const
{
  return this->GetId() == o->GetId();
}

//ImageDescriptor ViewDescriptor::GetImageDescriptor()
//{
//  if (imageDescriptor != null)
//  {
//    return imageDescriptor;
//  }
//  String iconName =
//      configElement.getAttribute(IWorkbenchRegistryConstants.ATT_ICON);
//  // If the icon attribute was omitted, use the default one
//  if (iconName == null)
//  {
//    return PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(ISharedImages.IMG_DEF_VIEW);
//  }
//  IExtension extension = configElement.getDeclaringExtension();
//  String extendingPluginId = extension.getNamespace();
//  imageDescriptor = AbstractUIPlugin.imageDescriptorFromPlugin(
//      extendingPluginId, iconName);
//  // If the icon attribute was invalid, use the error icon
//  if (imageDescriptor == null)
//  {
//    imageDescriptor = ImageDescriptor.getMissingImageDescriptor();
//  }
//
//  return imageDescriptor;
//}

std::string ViewDescriptor::GetLabel()
{
  std::string label;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, label);
  return label;
}

std::string ViewDescriptor::GetAccelerator()
{
  std::string accel;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ACCELERATOR, accel);
  return accel;
}

bool ViewDescriptor::GetAllowMultiple()
{
  std::string string;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ALLOW_MULTIPLE, string);
  if (string == "1") return true;
  Poco::toUpperInPlace(string);
  if (string == "TRUE") return true;
  return false;
}

void* ViewDescriptor::GetAdapter(const std::type_info& adapter)
{
  if (adapter == typeid(IConfigurationElement))
  {
    return GetConfigurationElement();
  }
  return 0;
}

//void
//ViewDescriptor::ActivateHandler() {
// if (handlerActivation == null) {
//   final IHandler handler = new ShowViewHandler(getId());
//   final IHandlerService handlerService = (IHandlerService) PlatformUI.getWorkbench().getService(IHandlerService.class);
//   handlerActivation = handlerService
//       .activateHandler(getId(), handler);
// }
// }

//void
//ViewDescriptor::DeactivateHandler() {
// if (handlerActivation != null) {
//   final IHandlerService handlerService = (IHandlerService) PlatformUI.getWorkbench().getService(IHandlerService.class);
//   handlerService.deactivateHandler(handlerActivation);
//   handlerActivation = null;
// }
//}


void ViewDescriptor::LoadFromExtension()
{
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id);

  // Sanity check.
  std::string name;
  if ((configElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, name) == false)
      || (RegistryReader::GetClassValue(configElement,
          WorkbenchRegistryConstants::ATT_CLASS) == ""))
  {
    throw CoreException(
            "Invalid extension (missing label or class name)", id);
  }

  std::string category;
  if (configElement->GetAttribute(WorkbenchRegistryConstants::TAG_CATEGORY, category))
  {
    Poco::StringTokenizer stok(category, "/", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
    // Parse the path tokens and store them
    for (Poco::StringTokenizer::Iterator iter = stok.begin(); iter != stok.end(); ++iter)
    {
      categoryPath.push_back(*iter);
    }
  }
}


} // namespace cherry
