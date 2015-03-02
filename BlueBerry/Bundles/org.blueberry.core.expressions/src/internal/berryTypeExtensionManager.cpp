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

#include "berryLog.h"

#include "berryTypeExtensionManager.h"

#include "berryExpressions.h"
#include "berryExpressionStatus.h"

#include "berryPropertyTesterDescriptor.h"

#include "berryPlatform.h"
#include "berryCoreException.h"
#include <berryIExtensionRegistry.h>

#include <ctime>

namespace berry {

  const QString TypeExtensionManager::TYPE= "type";

  TypeExtensionManager::TypeExtensionManager(const QString& extensionPoint)
  : fExtensionPoint(extensionPoint)
  {
    //Platform.getExtensionRegistry().addRegistryChangeListener(this);
    this->InitializeCaches();
  }

  Property::Pointer TypeExtensionManager::GetProperty(Object::ConstPointer receiver,
      const QString& namespaze, const QString& method)
  {
    return GetProperty(receiver, namespaze, method, false);
  }

  /*synchronized*/Property::Pointer
  TypeExtensionManager::GetProperty(
      Object::ConstPointer receiver, const QString& namespaze,
      const QString& method, bool forcePluginActivation)
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
          BERRY_INFO << "[Type Extension] - method "
                     << receiver->ToString() << "#" << method.toStdString()
                     << " found in cache: "
                     << (double(std::clock() - start))/(CLOCKS_PER_SEC/1000) << " ms.";
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
      QString msg("Unknown method for ");
      msg.append(receiver->GetClassName());
      IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::TYPE_EXTENDER_UNKOWN_METHOD,
                                                   QString("No property tester contributes a property %1 to type %2")
                                                   .arg(namespaze + "::" + method).arg(receiver->GetClassName()),
                                                   BERRY_STATUS_LOC));
      throw CoreException(status);
    }
    result->SetPropertyTester(extender);
    fPropertyCache->Put(result);
    if (Expressions::TRACING)
    {
      BERRY_INFO << "[Type Extension] - method "
                 << typeid(receiver).name() << "#" << method
                 << " not found in cache: "
                 << (double(std::clock() - start))/(CLOCKS_PER_SEC/1000) << " ms.";
    }
    return result;
  }

  /* package */TypeExtension::Pointer
  TypeExtensionManager::Get(const QString& type)
  {
    TypeExtension::Pointer result(fTypeExtensionMap[type]);
    if (result.IsNull())
    {
      result = new TypeExtension(type);
      fTypeExtensionMap[type] = result;
    }
    return result;
  }

  QList<IPropertyTester::Pointer> TypeExtensionManager::LoadTesters(const QString& typeName)
  {
    if (fConfigurationElementMap.isEmpty())
    {
      IExtensionRegistry* registry = Platform::GetExtensionRegistry();
      QList<IConfigurationElement::Pointer> ces(
            registry->GetConfigurationElementsFor(QString("org.blueberry.core.expressions.") + fExtensionPoint));
      for (int i= 0; i < ces.size(); i++)
      {
        IConfigurationElement::Pointer config(ces[i]);
        QString typeAttr = config->GetAttribute(TYPE);
        fConfigurationElementMap[typeAttr].push_back(config);
      }
    }
    //std::string typeName= type.getName();
    QList<IConfigurationElement::Pointer> typeConfigs = fConfigurationElementMap.take(typeName);

    QList<IPropertyTester::Pointer> result;
    for (int i= 0; i < typeConfigs.size(); i++)
    {
      IConfigurationElement::Pointer config(typeConfigs[i]);
      try
      {
        IPropertyTester::Pointer descr(new PropertyTesterDescriptor(config));
        result.push_back(descr);
      }
      catch (const CoreException& /*e*/)
      {
        //TODO
        //ExpressionPlugin.getDefault().getLog().log(e.getStatus());
        IPropertyTester::Pointer nullTester(new NULL_PROPERTY_TESTER_());
        result.push_back(nullTester);
      }
    }

    return result;
  }

  /*synchronized*/void TypeExtensionManager::InitializeCaches()
  {
    fPropertyCache = new PropertyCache(1000);
    fConfigurationElementMap.clear();
    fTypeExtensionMap.clear();
  }

  TypeExtensionManager::~TypeExtensionManager()
  {
    if (fPropertyCache) delete fPropertyCache;
  }

}
