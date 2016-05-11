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

#include "berryIConfigurationElement.h"
#include "berryCoreException.h"
#include "berryIExtension.h"
#include "berryIContributor.h"
#include "berryStatus.h"

#include "berryRegistryReader.h"
#include "berryWorkbenchRegistryConstants.h"

#include "berryAbstractUICTKPlugin.h"
#include "handlers/berryIHandlerActivation.h"

namespace berry
{

ViewDescriptor::ViewDescriptor(const IConfigurationElement::Pointer& e) :
  configElement(e)
{
  this->LoadFromExtension();
}

IViewPart::Pointer ViewDescriptor::CreateView()
{
  IViewPart::Pointer part(configElement->CreateExecutableExtension<IViewPart> (
      WorkbenchRegistryConstants::ATT_CLASS));
  return part;
}

QStringList ViewDescriptor::GetCategoryPath() const
{
  return categoryPath;
}

IConfigurationElement::Pointer ViewDescriptor::GetConfigurationElement() const
{
  return configElement;
}

QString ViewDescriptor::GetDescription() const
{
  return RegistryReader::GetDescription(configElement);
}

QString ViewDescriptor::GetId() const
{
  return id;
}

bool ViewDescriptor::operator==(const Object* o) const
{
  if (const IViewDescriptor* other = dynamic_cast<const IViewDescriptor*>(o))
  return this->GetId() == other->GetId();

  return false;
}

QIcon ViewDescriptor::GetImageDescriptor() const
{
  if (!imageDescriptor.isNull())
  {
    return imageDescriptor;
  }
  QString iconName = configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ICON);
  // If the icon attribute was omitted, use the default one
  if (iconName.isEmpty())
  {
    //TODO default image descriptor
    //return PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(ISharedImages.IMG_DEF_VIEW);
    return AbstractUICTKPlugin::GetMissingIcon();
  }
  IExtension::Pointer extension(configElement->GetDeclaringExtension());
  const QString extendingPluginId(extension->GetContributor()->GetName());
  imageDescriptor = AbstractUICTKPlugin::ImageDescriptorFromPlugin(
      extendingPluginId, iconName);

  // If the icon attribute was invalid, use the error icon
  if (imageDescriptor.isNull())
  {
    imageDescriptor = AbstractUICTKPlugin::GetMissingIcon();
  }

  return imageDescriptor;
}

QString ViewDescriptor::GetCase() const
{
  return configElement->GetAttribute(WorkbenchRegistryConstants::ATT_CASE);
}

QString ViewDescriptor::GetLabel() const
{
  return configElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME);
}

QString ViewDescriptor::GetAccelerator() const
{
  return configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ACCELERATOR);
}

bool ViewDescriptor::GetAllowMultiple() const
{
  return configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ALLOW_MULTIPLE).compare("true", Qt::CaseInsensitive) == 0;
}

bool ViewDescriptor::IsRestorable() const
{
  QString str = configElement->GetAttribute(WorkbenchRegistryConstants::ATT_RESTORABLE);
  return str.isNull() ? true : str.compare("true", Qt::CaseInsensitive) == 0;
}

Object* ViewDescriptor::GetAdapter(const QString& adapter) const
{
  if (adapter == qobject_interface_iid<IConfigurationElement*>())
  {
    return GetConfigurationElement().GetPointer();
  }
  return nullptr;
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

QStringList ViewDescriptor::GetKeywordReferences() const
{
  QStringList result;
  auto keywordRefs = configElement->GetChildren("keywordReference");
  for (auto keywordRefsIt = keywordRefs.begin();
       keywordRefsIt != keywordRefs.end(); ++keywordRefsIt) // iterate over all refs
  {
    result.push_back((*keywordRefsIt)->GetAttribute("id"));
  }
  return result;
}

QString ViewDescriptor::GetPluginId() const
{
  return configElement->GetContributor()->GetName();
}

QString ViewDescriptor::GetLocalId() const
{
  return this->GetId();
}

void ViewDescriptor::LoadFromExtension()
{
  id = configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ID);

  // Sanity check.
  QString name = configElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME);
  if (name.isEmpty() ||
      RegistryReader::GetClassValue(configElement, WorkbenchRegistryConstants::ATT_CLASS).isEmpty())
  {
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, configElement->GetContributor()->GetName(), nullptr,
                                       QString("Invalid extension (missing label or class name): ") + id));
    throw CoreException(status);
  }

  QString category = configElement->GetAttribute(WorkbenchRegistryConstants::TAG_CATEGORY);
  if (!category.isEmpty())
  {
    // Parse the path tokens and store them
    foreach (QString pathElement, category.split('/', QString::SkipEmptyParts))
    {
      if (!pathElement.trimmed().isEmpty())
      {
        categoryPath.push_back(pathElement.trimmed());
      }
    }
  }
}

} // namespace berry
