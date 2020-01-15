/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPerspectiveDescriptor.h"

#include "berryWorkbenchRegistryConstants.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchConstants.h"
#include "berryPerspectiveRegistry.h"
#include "berryStatus.h"
#include "berryIContributor.h"

#include <QIcon>

namespace berry
{

PerspectiveDescriptor::PerspectiveDescriptor(const QString& id,
    const QString& label, PerspectiveDescriptor::Pointer originalDescriptor)
 : singleton(false), fixed(false)
{
  this->id = id;
  this->label = label;
  if (originalDescriptor != 0)
  {
    this->originalId = originalDescriptor->GetOriginalId();
    this->imageDescriptor = originalDescriptor->imageDescriptor;

    // This perspective is based on a perspective in some bundle -- if
    // that
    // bundle goes away then I think it makes sense to treat this
    // perspective
    // the same as any other -- so store it with the original
    // descriptor's
    // bundle's list.
    //
    // It might also make sense the other way...removing the following
    // line
    // will allow the perspective to stay around when the originating
    // bundle
    // is unloaded.
    //
    // This might also have an impact on upgrade cases -- should we
    // really be
    // destroying all user customized perspectives when the older
    // version is
    // removed?
    //
    // I'm leaving this here for now since its a good example, but
    // wouldn't be
    // surprised if we ultimately decide on the opposite.
    //
    // The reason this line is important is that this is the value used
    // to
    // put the object into the UI level registry. When that bundle goes
    // away,
    // the registry will remove the entire list of objects. So if this
    // desc
    // has been put into that list -- it will go away.
    this->pluginId = originalDescriptor->GetPluginId();
  }
}

PerspectiveDescriptor::PerspectiveDescriptor(const QString& id,
    IConfigurationElement::Pointer configElement)
 : singleton(false), fixed(false)
{
  this->configElement = configElement;
  this->id = id;
  // Sanity check.
  if ((this->GetId() == "") || (this->GetLabel() == "")
      || (this->GetFactoryClassName() == ""))
  {
    IStatus::Pointer status(new Status(
                              IStatus::ERROR_TYPE,
                              PlatformUI::PLUGIN_ID(),
                              nullptr,
                              QString("Invalid extension (missing label, id or class name): ") + GetId()));
    throw CoreException(status);
  }
}

IPerspectiveFactory::Pointer PerspectiveDescriptor::CreateFactory()
{
  // if there is an originalId, then use that descriptor instead
  if (originalId != "")
  {
    // Get the original descriptor to create the factory. If the
    // original is gone then nothing can be done.
    IPerspectiveDescriptor::Pointer
        target =
            dynamic_cast<PerspectiveRegistry*> (WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry()) ->FindPerspectiveWithId(
                originalId);

    return target == 0 ? IPerspectiveFactory::Pointer(nullptr) : target.Cast<
        PerspectiveDescriptor> ()->CreateFactory();
  }

  // otherwise try to create the executable extension
  if (configElement != 0)
  {
    try
    {
      IPerspectiveFactory::Pointer factory(
          configElement ->CreateExecutableExtension<IPerspectiveFactory> (
              WorkbenchRegistryConstants::ATT_CLASS));
      return factory;
    }
    catch (const CoreException& /*e*/)
    {
      // do nothing
    }
  }

  return IPerspectiveFactory::Pointer(nullptr);
}

void PerspectiveDescriptor::DeleteCustomDefinition()
{
  dynamic_cast<PerspectiveRegistry*> (WorkbenchPlugin::GetDefault() ->GetPerspectiveRegistry())->DeleteCustomDefinition(
      PerspectiveDescriptor::Pointer(this));
}

QString PerspectiveDescriptor::GetDescription() const
{
  return configElement == 0 ? description : RegistryReader::GetDescription(
      configElement);
}

void PerspectiveDescriptor::SetDescription(const QString& desc)
{
  description = desc;
}

bool PerspectiveDescriptor::GetFixed() const
{
  if (configElement == 0)
    return fixed;

  return configElement->GetAttribute(WorkbenchRegistryConstants::ATT_FIXED).compare("true", Qt::CaseInsensitive) == 0;
}

QStringList PerspectiveDescriptor::GetKeywordReferences() const
{
  QStringList result;
  if (configElement.IsNull())
  {
    return result;
  }

  auto keywordRefs = configElement->GetChildren("keywordReference");
  for (auto keywordRefsIt = keywordRefs.begin();
       keywordRefsIt != keywordRefs.end(); ++keywordRefsIt) // iterate over all refs
  {
    result.push_back((*keywordRefsIt)->GetAttribute("id"));
  }
  return result;
}

QString PerspectiveDescriptor::GetId() const
{
  return id;
}

QString PerspectiveDescriptor::GetLocalId() const
{
  return GetId();
}

QString PerspectiveDescriptor::GetPluginId() const
{
  return configElement == 0 ? pluginId : configElement->GetContributor()->GetName();
}

QIcon PerspectiveDescriptor::GetImageDescriptor() const
{
  if (!imageDescriptor.isNull())
    return imageDescriptor;

  if (configElement)
  {
    QString icon = configElement->GetAttribute(WorkbenchRegistryConstants::ATT_ICON);
    if (!icon.isEmpty())
    {
      imageDescriptor = AbstractUICTKPlugin::ImageDescriptorFromPlugin(
            configElement->GetContributor()->GetName(), icon);
    }

  }

  if (imageDescriptor.isNull())
  {
    imageDescriptor = AbstractUICTKPlugin::GetMissingIcon();
  }

  return imageDescriptor;
}

QStringList PerspectiveDescriptor::GetCategoryPath() const
{
  if(!categoryPath.empty()) return categoryPath;

  if (configElement.IsNotNull())
  {
    QString category = configElement->GetAttribute(WorkbenchRegistryConstants::TAG_CATEGORY);
    categoryPath = category.split('/', QString::SkipEmptyParts);
  }
  return categoryPath;
}

QString PerspectiveDescriptor::GetLabel() const
{
  if (configElement == 0)
    return label;

  return configElement->GetAttribute(WorkbenchRegistryConstants::ATT_NAME);
}

QString PerspectiveDescriptor::GetOriginalId() const
{
  if (originalId == "")
  {
    return this->GetId();
  }
  return originalId;
}

bool PerspectiveDescriptor::HasCustomDefinition() const
{
  return dynamic_cast<PerspectiveRegistry*> (WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry())->HasCustomDefinition(
      PerspectiveDescriptor::ConstPointer(this));
}

bool PerspectiveDescriptor::HasDefaultFlag() const
{
  if (configElement == 0)
  {
    return false;
  }

  return configElement->GetAttribute(WorkbenchRegistryConstants::ATT_DEFAULT).compare("true", Qt::CaseInsensitive) == 0;
}

bool PerspectiveDescriptor::IsPredefined() const
{
  return this->GetFactoryClassName() != "" && configElement != 0;
}

bool PerspectiveDescriptor::IsSingleton() const
{
  if (configElement == 0)
    return singleton;

  return configElement->GetAttribute(WorkbenchRegistryConstants::ATT_SINGLETON).compare("true", Qt::CaseInsensitive) == 0;
}

bool PerspectiveDescriptor::RestoreState(IMemento::Pointer memento)
{
  IMemento::Pointer childMem(memento->GetChild(
      WorkbenchConstants::TAG_DESCRIPTOR));
  if (childMem)
  {
    childMem->GetString(WorkbenchConstants::TAG_ID, id);
    childMem->GetString(WorkbenchConstants::TAG_DESCRIPTOR, originalId);
    childMem->GetString(WorkbenchConstants::TAG_LABEL, label);
    childMem->GetString(WorkbenchConstants::TAG_CLASS, className);
    int singletonVal;
    singleton = childMem->GetInteger(WorkbenchConstants::TAG_SINGLETON,
        singletonVal);

    // Find a descriptor in the registry.
    IPerspectiveDescriptor::Pointer
        descriptor =
            WorkbenchPlugin::GetDefault() ->GetPerspectiveRegistry()->FindPerspectiveWithId(
                this->GetOriginalId());

    if (descriptor)
    {
      // Copy the state from the registred descriptor.
      imageDescriptor = descriptor->GetImageDescriptor();
    }
  }
  //return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
  return true;
}

void PerspectiveDescriptor::RevertToPredefined()
{
  if (this->IsPredefined())
  {
    this->DeleteCustomDefinition();
  }
}

bool PerspectiveDescriptor::SaveState(IMemento::Pointer memento)
{
  IMemento::Pointer childMem(memento->CreateChild(
      WorkbenchConstants::TAG_DESCRIPTOR));
  childMem->PutString(WorkbenchConstants::TAG_ID, GetId());
  if (!originalId.isEmpty())
  {
    childMem->PutString(WorkbenchConstants::TAG_DESCRIPTOR, originalId);
  }
  childMem->PutString(WorkbenchConstants::TAG_LABEL, GetLabel());
  childMem->PutString(WorkbenchConstants::TAG_CLASS, GetFactoryClassName());
  if (singleton)
  {
    childMem->PutInteger(WorkbenchConstants::TAG_SINGLETON, 1);
  }
  //return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null);
  return true;
}

IConfigurationElement::Pointer PerspectiveDescriptor::GetConfigElement() const
{
  return configElement;
}

QString PerspectiveDescriptor::GetFactoryClassName() const
{
  return configElement == 0 ? className : RegistryReader::GetClassValue(
      configElement, WorkbenchRegistryConstants::ATT_CLASS);
}

}
