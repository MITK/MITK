/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLog.h"

#include "berryWorkbenchServiceRegistry.h"

#include "berryISources.h"
#include "berryISourceProvider.h"
#include "berryPlatformUI.h"

#include "services/berryIServiceFactory.h"

#include "berryServiceLocator.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchRegistryConstants.h"
#include "berrySourcePriorityNameMapping.h"
#include "berryAbstractSourceProvider.h"
#include "berryStatus.h"
#include "berryPlatformUI.h"

namespace berry
{

const QString WorkbenchServiceRegistry::WORKBENCH_LEVEL = "workbench";

const QString WorkbenchServiceRegistry::EXT_ID_SERVICES =
    "org.blueberry.ui.services"; //$NON-NLS-1$

const IServiceLocator::Pointer WorkbenchServiceRegistry::GLOBAL_PARENT =
    IServiceLocator::Pointer(new GlobalParentLocator());

QStringList WorkbenchServiceRegistry::SupportedLevels()
{
  struct _Levels {
    QStringList levels;
    _Levels()
    {
      levels << ISources::ACTIVE_CONTEXT_NAME()
             << ISources::ACTIVE_SHELL_NAME()
             << ISources::ACTIVE_WORKBENCH_WINDOW_NAME()
             << ISources::ACTIVE_EDITOR_ID_NAME()
             << ISources::ACTIVE_PART_ID_NAME()
             << ISources::ACTIVE_SITE_NAME();
    }
  };
  static _Levels _levels;

  return _levels.levels;
}

WorkbenchServiceRegistry::WorkbenchServiceRegistry()
{
//  PlatformUI::GetWorkbench().getExtensionTracker().registerHandler(this,
//      ExtensionTracker .createExtensionPointFilter(getExtensionPoint()));
}

WorkbenchServiceRegistry::ServiceFactoryHandle::ServiceFactoryHandle(
    const IServiceFactory* _factory)
: factory(_factory)
{

}

WorkbenchServiceRegistry::ServiceFactoryHandle::Pointer WorkbenchServiceRegistry::LoadFromRegistry(
    const QString& key)
{
  ServiceFactoryHandle::Pointer result;
  const QList<IConfigurationElement::Pointer> serviceFactories =
      this->GetExtensionPoint()->GetConfigurationElements();
  try
  {
    bool done = false;
    for (int i = 0; i < serviceFactories.size() && !done; i++)
    {
      QList<IConfigurationElement::Pointer> serviceNameElements =
          serviceFactories[i]->GetChildren(
              WorkbenchRegistryConstants::TAG_SERVICE);
      for (int j = 0; j < serviceNameElements.size() && !done; j++)
      {
        QString serviceName = serviceNameElements[j]->GetAttribute(
              WorkbenchRegistryConstants::ATTR_SERVICE_CLASS);
        if (key == serviceName)
        {
          done = true;
        }
      }
      if (done)
      {
        IServiceFactory* f = serviceFactories[i]->CreateExecutableExtension<IServiceFactory>(
              WorkbenchRegistryConstants::ATTR_FACTORY_CLASS);
        ServiceFactoryHandle::Pointer handle(new ServiceFactoryHandle(f));
//        PlatformUI.getWorkbench().getExtensionTracker().registerObject(
//            serviceFactories[i].getDeclaringExtension(), handle,
//            IExtensionTracker.REF_WEAK);

        QList<QString> serviceNames;
        for (int j = 0; j < serviceNameElements.size(); j++)
        {
          QString serviceName = serviceNameElements[j]->GetAttribute(
                WorkbenchRegistryConstants::ATTR_SERVICE_CLASS);
          if (factories.find(serviceName) != factories.end())
          {
            WorkbenchPlugin::Log("Factory already exists for "  + serviceName);
          }
          else
          {
            factories.insert(serviceName, handle);
            serviceNames.push_back(serviceName);
          }
        }
        handle->serviceNames = serviceNames;
        result = handle;
      }
    }
  } catch (const CoreException& e)
  {
    //StatusManager.getManager().handle(e.getStatus());
    BERRY_ERROR << "CoreException: " << e.what() << std::endl;
  }
  return result;
}

IExtensionPoint::Pointer WorkbenchServiceRegistry::GetExtensionPoint() const
{
  IExtensionRegistry* reg = Platform::GetExtensionRegistry();
  return reg->GetExtensionPoint(EXT_ID_SERVICES);
}

void WorkbenchServiceRegistry::ProcessVariables(
    const QList<IConfigurationElement::Pointer>& children) const
{
  for (int i = 0; i < children.size(); i++)
  {
    QString name = children[i]->GetAttribute(WorkbenchRegistryConstants::ATT_NAME);
    if (name.isEmpty())
    {
      continue;
    }
    QString level = children[i]->GetAttribute(WorkbenchRegistryConstants::ATT_PRIORITY_LEVEL);
    if (level.isEmpty())
    {
      level = WORKBENCH_LEVEL;
    }
    else
    {
      bool found = false;
      QStringList supportedLevels = this->SupportedLevels();
      for (int j = 0; j < supportedLevels.size() && !found; j++)
      {
        if (supportedLevels[j] == level)
        {
          found = true;
        }
      }
      if (!found)
      {
        level = WORKBENCH_LEVEL;
      }
    }
    int existingPriority = SourcePriorityNameMapping::GetMapping(level);
    int newPriority = existingPriority << 1;
    SourcePriorityNameMapping::AddMapping(name, newPriority);
  }
}

Object* WorkbenchServiceRegistry::GlobalParentLocator::GetService(
    const QString& /*api*/)
{
  return nullptr;
}

bool WorkbenchServiceRegistry::GlobalParentLocator::HasService(
    const QString& /*api*/) const
{
  return false;
}

WorkbenchServiceRegistry* WorkbenchServiceRegistry::GetRegistry()
{
  static WorkbenchServiceRegistry registry;
  return &registry;
}

Object::Pointer WorkbenchServiceRegistry::GetService(const QString& key,
    IServiceLocator* parentLocator, ServiceLocator* locator)
{
  ServiceFactoryHandle::Pointer handle = factories.value(key);
  if (!handle)
  {
    handle = this->LoadFromRegistry(key);
  }
  if (handle)
  {
    Object::Pointer result(handle->factory->Create(key, parentLocator, locator));
    if (result)
    {
      //handle->serviceLocators.insert(locator, new Object());
      return result;
    }
  }
  return Object::Pointer(nullptr);
}

QList<AbstractSourceProvider::Pointer> WorkbenchServiceRegistry::GetSourceProviders() const
{
  QList<AbstractSourceProvider::Pointer> providers;
  IExtensionPoint::Pointer ep = this->GetExtensionPoint();
  QList<IConfigurationElement::Pointer> elements = ep->GetConfigurationElements();
  for (int i = 0; i < elements.size(); i++)
  {
    if (elements[i]->GetName() == WorkbenchRegistryConstants::TAG_SOURCE_PROVIDER)
    {
      try
      {
        ISourceProvider* sourceProvider(elements[i]->CreateExecutableExtension<ISourceProvider>(
                                          WorkbenchRegistryConstants::ATTR_PROVIDER));
        if (AbstractSourceProvider* asp = dynamic_cast<AbstractSourceProvider*>(sourceProvider))
        {
          providers.push_back(AbstractSourceProvider::Pointer(asp));
        }
        else
        {
          const QString attributeName = elements[i]->GetAttribute(WorkbenchRegistryConstants::ATTR_PROVIDER);
          const QString message = "Source Provider '" + attributeName +
              "' should extend AbstractSourceProvider";
          const IStatus::Pointer status(new Status(IStatus::ERROR_TYPE,
                                                   PlatformUI::PLUGIN_ID(), message, BERRY_STATUS_LOC));
          WorkbenchPlugin::Log(status);
          continue;
        }
      }
      catch (const CoreException& e)
      {
        //StatusManager.getManager().handle(e.getStatus());
        BERRY_ERROR << "CoreException: " << e.what() << std::endl;
      }
    }
  }
  return providers;
}

void WorkbenchServiceRegistry::InitializeSourcePriorities()
{
  IExtensionPoint::Pointer ep = GetExtensionPoint();
  QList<IConfigurationElement::Pointer> elements = ep->GetConfigurationElements();
  for (int i = 0; i < elements.size(); i++)
  {
    if (elements[i]->GetName() == WorkbenchRegistryConstants::TAG_SOURCE_PROVIDER)
    {
      ProcessVariables(elements[i]->GetChildren(WorkbenchRegistryConstants::TAG_VARIABLE));
    }
  }
}

}
