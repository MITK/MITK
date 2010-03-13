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

#include "berryLog.h"

#include "berryWorkbenchServiceRegistry.h"

#include "../berryISources.h"
#include "../berryISourceProvider.h"
#include "../berryPlatformUI.h"
#include "../berryImageDescriptor.h"

#include "../services/berryIServiceFactory.h"

#include "berryServiceLocator.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchRegistryConstants.h"
#include "berrySourcePriorityNameMapping.h"

#include <berryIExtensionPointService.h>

namespace berry
{

const std::string WorkbenchServiceRegistry::WORKBENCH_LEVEL = "workbench"; //$NON-NLS-1$

const std::string WorkbenchServiceRegistry::EXT_ID_SERVICES =
    "org.blueberry.ui.services"; //$NON-NLS-1$

WorkbenchServiceRegistry* WorkbenchServiceRegistry::registry = 0;

const IServiceLocator::Pointer WorkbenchServiceRegistry::GLOBAL_PARENT =
    IServiceLocator::Pointer(new GlobalParentLocator());

std::string* WorkbenchServiceRegistry::supportedLevels()
{
  static std::string levels[] = {
  ISources::ACTIVE_CONTEXT_NAME(),
  ISources::ACTIVE_SHELL_NAME(),
  ISources::ACTIVE_WORKBENCH_WINDOW_NAME(),
  ISources::ACTIVE_EDITOR_ID_NAME(),
  ISources::ACTIVE_PART_ID_NAME(),
  ISources::ACTIVE_SITE_NAME() };

  return levels;
}

const unsigned int WorkbenchServiceRegistry::supportedLevelsCount = 6;

WorkbenchServiceRegistry::WorkbenchServiceRegistry()
{
//  PlatformUI::GetWorkbench().getExtensionTracker().registerHandler(this,
//      ExtensionTracker .createExtensionPointFilter(getExtensionPoint()));
}

WorkbenchServiceRegistry::ServiceFactoryHandle::ServiceFactoryHandle(
    IServiceFactory::ConstPointer _factory)
: factory(_factory)
{

}

WorkbenchServiceRegistry::ServiceFactoryHandle::Pointer WorkbenchServiceRegistry::LoadFromRegistry(
    const std::string& key)
{
  ServiceFactoryHandle::Pointer result;
  const std::vector<IConfigurationElement::Pointer> serviceFactories =
      this->GetExtensionPoint()->GetConfigurationElements();
  try
  {
    bool done = false;
    for (unsigned int i = 0; i < serviceFactories.size() && !done; i++)
    {
      std::vector<IConfigurationElement::Pointer> serviceNameElements =
          serviceFactories[i]->GetChildren(
              WorkbenchRegistryConstants::TAG_SERVICE);
      for (unsigned int j = 0; j < serviceNameElements.size() && !done; j++)
      {
        std::string serviceName;
        serviceNameElements[j]->GetAttribute(
            WorkbenchRegistryConstants::ATTR_SERVICE_CLASS, serviceName);
        if (key == serviceName)
        {
          done = true;
        }
      }
      if (done)
      {
        IServiceFactory::Pointer f(
                serviceFactories[i]->CreateExecutableExtension<IServiceFactory>(
                    WorkbenchRegistryConstants::ATTR_FACTORY_CLASS));
        ServiceFactoryHandle::Pointer handle(new ServiceFactoryHandle(f));
//        PlatformUI.getWorkbench().getExtensionTracker().registerObject(
//            serviceFactories[i].getDeclaringExtension(), handle,
//            IExtensionTracker.REF_WEAK);

        std::vector<std::string> serviceNames;
        for (unsigned int j = 0; j < serviceNameElements.size(); j++)
        {
          std::string serviceName;
          serviceNameElements[j]->GetAttribute(
              WorkbenchRegistryConstants::ATTR_SERVICE_CLASS, serviceName);
          if (factories.find(serviceName) != factories.end())
          {
            WorkbenchPlugin::Log("Factory already exists for "  + serviceName);
          }
          else
          {
            factories.insert(std::make_pair(serviceName, handle));
            serviceNames.push_back(serviceName);
          }
        }
        handle->serviceNames = serviceNames;
        result = handle;
      }
    }
  } catch (CoreException& e)
  {
    //StatusManager.getManager().handle(e.getStatus());
    BERRY_ERROR << "CoreException: " << e.displayText() << std::endl;
  }
  return result;
}

const IExtensionPoint* WorkbenchServiceRegistry::GetExtensionPoint()
{
  IExtensionPointService::Pointer reg = Platform::GetExtensionPointService();
  const IExtensionPoint* ep = reg->GetExtensionPoint(EXT_ID_SERVICES);
  return ep;
}

void WorkbenchServiceRegistry::ProcessVariables(
    const std::vector<IConfigurationElement::Pointer>& children)
{
  for (unsigned int i = 0; i < children.size(); i++)
  {
    std::string name;
    children[i]->GetAttribute(WorkbenchRegistryConstants::ATT_NAME, name);
    if (name.empty())
    {
      continue;
    }
    std::string level;
    children[i]->GetAttribute(WorkbenchRegistryConstants::ATT_PRIORITY_LEVEL, level);
    if (level.empty())
    {
      level = WORKBENCH_LEVEL;
    }
    else
    {
      bool found = false;
      const std::string* const supportedLevels = this->supportedLevels();
      for (unsigned int j = 0; j < supportedLevelsCount && !found; j++)
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

Object::Pointer WorkbenchServiceRegistry::GlobalParentLocator::GetService(
    const std::string& /*api*/)
{
  return Object::Pointer(0);
}

bool WorkbenchServiceRegistry::GlobalParentLocator::HasService(
    const std::string& /*api*/) const
{
  return false;
}

WorkbenchServiceRegistry* WorkbenchServiceRegistry::GetRegistry()
{
  if (registry == 0)
  {
    registry = new WorkbenchServiceRegistry();
  }
  return registry;
}

Object::Pointer WorkbenchServiceRegistry::GetService(const std::string& key,
    const IServiceLocator::Pointer parentLocator, const ServiceLocator::ConstPointer locator)
{
  ServiceFactoryHandle::Pointer handle(factories[key]);
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
  return Object::Pointer(0);
}

std::vector<ISourceProvider::Pointer> WorkbenchServiceRegistry::GetSourceProviders()
{
  std::vector<ISourceProvider::Pointer> providers;
  const IExtensionPoint* ep = this->GetExtensionPoint();
  std::vector<IConfigurationElement::Pointer> elements =
      ep->GetConfigurationElements();
  for (unsigned int i = 0; i < elements.size(); i++)
  {
    if (elements[i]->GetName() ==
        WorkbenchRegistryConstants::TAG_SOURCE_PROVIDER)
    {
      try
      {
        ISourceProvider::Pointer provider(elements[i]->CreateExecutableExtension<ISourceProvider>(
            WorkbenchRegistryConstants::ATTR_PROVIDER));
        providers.push_back(provider);
        this->ProcessVariables(elements[i]->GetChildren(
            WorkbenchRegistryConstants::TAG_VARIABLE));
      } catch (CoreException& e)
      {
        //StatusManager.getManager().handle(e.getStatus());
        BERRY_ERROR << "CoreException: " << e.displayText() << std::endl;
      }
    }
  }
  return providers;
}

}
