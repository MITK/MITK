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

#include "cherryTypeExtensionManager.h"

#include "cherryExpressions.h"

#include "cherryPropertyTesterDescriptor.h"

#include "cherryPlatform.h"
#include "service/cherryIExtensionPointService.h"

#include <ctime>

namespace cherry {

  const std::string TypeExtensionManager::TYPE= "type"; //$NON-NLS-1$

  TypeExtensionManager::TypeExtensionManager(const std::string& extensionPoint)
  : fExtensionPoint(extensionPoint)
  {
    //Platform.getExtensionRegistry().addRegistryChangeListener(this);
    this->InitializeCaches();
  }

  Property::Pointer TypeExtensionManager::GetProperty(Object::Pointer receiver,
      const std::string& namespaze, const std::string& method)
  {
    return GetProperty(receiver, namespaze, method, false);
  }

  /*synchronized*/Property::Pointer
  TypeExtensionManager::GetProperty(
      Object::Pointer receiver, const std::string& namespaze,
      const std::string& method, bool forcePluginActivation)
  {
    std::clock_t start= 0;
    if (Expressions::TRACING)
    start = std::clock();

    // if we call a static method than the receiver is the class object
    //Class clazz= receiver instanceof Class ? (Class)receiver : receiver.getClass();

    Property::Pointer result(new Property(receiver, namespaze, method));
    Property::Pointer cached(fPropertyCache->Get(result));
    if (!cached.isNull())
    {
      if (cached->IsValidCacheEntry(forcePluginActivation))
      {
        if (Expressions::TRACING)
        {
          std::cout << "[Type Extension] - method " <<
          receiver->ToString() << "#" << method <<
          " found in cache: " <<
          (double(std::clock() - start))/(CLOCKS_PER_SEC/1000) << " ms.";
        }
        return cached;
      }
      // The type extender isn't loaded in the cached method but can be loaded
      // now. So remove method from cache and do the normal look up so that the
      // implementation class gets loaded.
      fPropertyCache->Remove(cached);
    }
    TypeExtension::Pointer extension(this->Get(receiver->GetClassName()));
    IPropertyTester::Pointer extender(extension->FindTypeExtender(*this, namespaze, method, false /*receiver instanceof Class*/, forcePluginActivation));
    if (!extender.Cast<TypeExtension::CONTINUE_>().IsNull() || extender.IsNull())
    {
      std::string msg("Unknown method for ");
      msg.append(receiver->GetClassName());
      throw CoreException(msg, method);
    }
    result->SetPropertyTester(extender);
    fPropertyCache->Put(result);
    if (Expressions::TRACING)
    {
      std::cout << "[Type Extension] - method " <<
      typeid(receiver).name() << "#" << method <<
      " not found in cache: " <<
      (double(std::clock() - start))/(CLOCKS_PER_SEC/1000) << " ms.";
    }
    return result;
  }

  /* package */TypeExtension::Pointer
  TypeExtensionManager::Get(const std::string& type)
  {
    TypeExtension::Pointer result(fTypeExtensionMap[type]);
    if (result.IsNull())
    {
      result = new TypeExtension(type);
      fTypeExtensionMap[type] = result;
    }
    return result;
  }

  /* package */void TypeExtensionManager::LoadTesters(
      std::vector<IPropertyTester::Pointer>& result, const std::string& typeName)
  {
    if (fConfigurationElementMap == 0)
    {
      fConfigurationElementMap = new std::map<std::string, std::vector<IConfigurationElement::Pointer> >();
      IExtensionPointService::Pointer registry(Platform::GetExtensionPointService());
      IConfigurationElement::vector ces(
        registry->GetConfigurationElementsFor("org.opencherry.core.expressions." + fExtensionPoint));
      for (unsigned int i= 0; i < ces.size(); i++)
      {
        IConfigurationElement::Pointer config(ces[i]);
        std::string typeAttr;
        config->GetAttribute(TYPE, typeAttr);
        std::vector<IConfigurationElement::Pointer> typeConfigs = (*fConfigurationElementMap)[typeAttr];
        typeConfigs.push_back(config);
      }
    }
    //std::string typeName= type.getName();
    std::vector<IConfigurationElement::Pointer> typeConfigs = (*fConfigurationElementMap)[typeName];

    for (unsigned int i= 0; i < typeConfigs.size(); i++)
    {
      IConfigurationElement::Pointer config(typeConfigs[i]);
      try
      {
        IPropertyTester::Pointer descr(new PropertyTesterDescriptor(config));
        result.push_back(descr);
      }
      catch (CoreException e)
      {
        //TODO
        //ExpressionPlugin.getDefault().getLog().log(e.getStatus());
        IPropertyTester::Pointer nullTester(new NULL_PROPERTY_TESTER_());
        result.push_back(nullTester);
      }
    }
    fConfigurationElementMap->erase(typeName);

  }

  /*synchronized*/void TypeExtensionManager::InitializeCaches()
  {
    fPropertyCache = new PropertyCache(1000);
    fConfigurationElementMap = 0;
  }

  TypeExtensionManager::~TypeExtensionManager()
  {
    if (fPropertyCache) delete fPropertyCache;
    if (fConfigurationElementMap) delete fConfigurationElementMap;
  }

}
