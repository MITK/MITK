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

#include "berryViewDescriptor.h"

#include "service/berryIConfigurationElement.h"
#include "berryPlatformException.h"

#include "berryRegistryReader.h"
#include "berryWorkbenchRegistryConstants.h"

#include "berryImageDescriptor.h"
#include "berryAbstractUIPlugin.h"
#include "berryAbstractUICTKPlugin.h"
#include "berryImageDescriptor.h"
#include "handlers/berryIHandlerActivation.h"

#include <Poco/String.h>
#include <Poco/StringTokenizer.h>

namespace berry
{
ViewDescriptor::ViewDescriptor(IConfigurationElement::Pointer e) :
  configElement(e)
{
  this->LoadFromExtension();
}

IViewPart::Pointer ViewDescriptor::CreateView()
{
  IViewPart::Pointer part(configElement->CreateExecutableExtension<IViewPart> (
      WorkbenchRegistryConstants::ATT_CLASS));
  if (part.IsNull())
  {
    // support legacy BlueBerry extensions
    part = configElement->CreateExecutableExtension<IViewPart> (
          WorkbenchRegistryConstants::ATT_CLASS, IViewPart::GetManifestName());
  }
  return part;
}

const std::vector<std::string>& ViewDescriptor::GetCategoryPath() const
{
  return categoryPath;
}

IConfigurationElement::Pointer ViewDescriptor::GetConfigurationElement() const
{
  return configElement;
}

std::string ViewDescriptor::GetDescription() const
{
  return RegistryReader::GetDescription(configElement);
}

std::string ViewDescriptor::GetId() const
{
  return id;
}

bool ViewDescriptor::operator==(const Object* o) const
{
  if (const IViewDescriptor* other = dynamic_cast<const IViewDescriptor*>(o))
  return this->GetId() == other->GetId();

  return false;
}

ImageDescriptor::Pointer ViewDescriptor::GetImageDescriptor() const
{
  if (imageDescriptor)
  {
    return imageDescriptor;
  }
  std::string iconName;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ICON, iconName);
  // If the icon attribute was omitted, use the default one
  if (iconName.empty())
  {
    //TODO default image descriptor
    //return PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(ISharedImages.IMG_DEF_VIEW);
    return ImageDescriptor::GetMissingImageDescriptor();
  }
  const IExtension* extension(configElement->GetDeclaringExtension());
  const std::string extendingPluginId(extension->GetNamespace());
  imageDescriptor = AbstractUICTKPlugin::ImageDescriptorFromPlugin(
      extendingPluginId, iconName);
  if (!imageDescriptor)
  {
    // Try legacy BlueBerry method
    imageDescriptor = AbstractUIPlugin::ImageDescriptorFromPlugin(
      extendingPluginId, iconName);
  }

  // If the icon attribute was invalid, use the error icon
  if (!imageDescriptor)
  {
    imageDescriptor = ImageDescriptor::GetMissingImageDescriptor();
  }

  return imageDescriptor;
}

std::string ViewDescriptor::GetLabel() const
{
  std::string label;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, label);
  return label;
}

std::string ViewDescriptor::GetAccelerator() const
{
  std::string accel;
  configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ACCELERATOR, accel);
  return accel;
}

bool ViewDescriptor::GetAllowMultiple() const
{
  bool allow = false;
  configElement->GetBoolAttribute(WorkbenchRegistryConstants::ATT_ALLOW_MULTIPLE, allow);
  return allow;
}

bool ViewDescriptor::IsRestorable() const {
  std::string string;
  if (configElement->GetAttribute(WorkbenchRegistryConstants::ATT_RESTORABLE, string))
  {
    return Poco::icompare(string, "true") == 0;
  }
  else {
    return true;
  }
}

Poco::Any ViewDescriptor::GetAdapter(const std::string& adapter)
{
  if (adapter == IConfigurationElement::GetStaticClassName())
  {
    return Poco::Any(GetConfigurationElement());
  }
  return Poco::Any();
}

void
ViewDescriptor::ActivateHandler()
{
  //TODO ViewDescriptor handler activation
//  if (!handlerActivation)
//  {
//    IHandler::Pointer handler(new ShowViewHandler(this->GetId()));
//    IHandlerService::Pointer handlerService(
//        PlatformUI::GetWorkbench()->GetService(IHandlerService::GetManifestName()).Cast<IHandlerService>());
//    handlerActivation = handlerService
//    ->ActivateHandler(this->GetId(), handler);
//  }
}

void
ViewDescriptor::DeactivateHandler()
{
  //TODO ViewDescriptor handler deactivation
//  if (handlerActivation)
//  {
//    IHandlerService::Pointer handlerService(
//        PlatformUI::GetWorkbench()->GetService(IHandlerService::GetManifestName()).Cast<IHandlerService>());
//    handlerService->DeactivateHandler(handlerActivation);
//    handlerActivation = 0;
//  }
}

std::vector< std::string> ViewDescriptor::GetKeywordReferences() const
{
  std::vector<std::string> result;
  std::string keywordRefId;
  std::vector<berry::IConfigurationElement::Pointer> keywordRefs;
  berry::IConfigurationElement::vector::iterator keywordRefsIt;
  keywordRefs = configElement->GetChildren("keywordReference");
  for (keywordRefsIt = keywordRefs.begin()
          ; keywordRefsIt != keywordRefs.end(); ++keywordRefsIt) // iterate over all refs
  {
    (*keywordRefsIt)->GetAttribute("id", keywordRefId);
    result.push_back(keywordRefId);
  }
  return result;
}


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
} // namespace berry