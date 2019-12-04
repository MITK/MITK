/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryConfigurationElement.h"

#include "berryIExecutableExtension.h"
#include "berryIExecutableExtensionFactory.h"
#include "berryConfigurationElementHandle.h"
#include "berryStatus.h"
#include "berryRegistryMessages.h"
#include "berryRegistryConstants.h"
#include "berryCoreException.h"
#include "berryExtensionRegistry.h"
#include "berryRegistryObjectManager.h"
#include "berryRegistryContributor.h"
#include "berryObjectString.h"
#include "berryObjectStringMap.h"

#include <QRegExp>

namespace berry {

ConfigurationElement::ConfigurationElement(ExtensionRegistry* registry, bool persist)
  : RegistryObject(registry, persist)
{
}

ConfigurationElement::ConfigurationElement(int self, const QString& contributorId,
                                           const QString& name, const QList<QString>& propertiesAndValue,
                                           const QList<int>& children, int extraDataOffset, int parent,
                                           short parentType, ExtensionRegistry* registry, bool persist)
  : RegistryObject(registry, persist), parentId(parent), parentType(parentType),
    propertiesAndValue(propertiesAndValue), name(name), contributorId(contributorId)
{
  SetObjectId(self);
  SetRawChildren(children);
  SetExtraDataOffset(extraDataOffset);
}

void ConfigurationElement::ThrowException(const QString& message, const ctkException& exc)
{
  IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME,
                                     RegistryConstants::PLUGIN_ERROR, message, exc,
                                     BERRY_STATUS_LOC));
  throw CoreException(status);
}

void ConfigurationElement::ThrowException(const QString &message)
{
  IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME,
                                     RegistryConstants::PLUGIN_ERROR, message,
                                     BERRY_STATUS_LOC));
  throw CoreException(status);
}

QString ConfigurationElement::GetValue() const
{
  return GetValueAsIs();
}

QString ConfigurationElement::GetValueAsIs() const
{
  if (!propertiesAndValue.empty() && propertiesAndValue.size() % 2 == 1)
    return propertiesAndValue.back();
  return QString();
}

QString ConfigurationElement::GetAttributeAsIs(const QString& attrName) const
{
  if (propertiesAndValue.size() <= 1)
    return QString();
  int size = propertiesAndValue.size() - (propertiesAndValue.size() % 2);
  for (int i = 0; i < size; i += 2)
  {
    if (propertiesAndValue[i] == attrName)
      return propertiesAndValue[i + 1];
  }
  return QString();
}

QList<QString> ConfigurationElement::GetAttributeNames() const
{
  if (propertiesAndValue.size() <= 1)
    return QList<QString>();

  QList<QString> result;
  int size = propertiesAndValue.size() / 2;
  for (int i = 0; i < size; i++)
  {
    result.push_back(propertiesAndValue[i * 2]);
  }
  return result;
}

void ConfigurationElement::SetProperties(const QList<QString>& value)
{
  propertiesAndValue = value;
}

QList<QString> ConfigurationElement::GetPropertiesAndValue() const
{
  return propertiesAndValue;
}

void ConfigurationElement::SetValue(const QString& value)
{
  if (propertiesAndValue.empty())
  {
    propertiesAndValue.push_back(value);
    return;
  }
  if (propertiesAndValue.size() % 2 == 1)
  {
    propertiesAndValue[propertiesAndValue.size() - 1] = value;
    return;
  }
  propertiesAndValue.push_back(value);
}

void ConfigurationElement::SetContributorId(const QString& id)
{
  contributorId = id;
}

QString ConfigurationElement::GetContributorId() const
{
  return contributorId;
}

void ConfigurationElement::SetParentId(int objectId)
{
  parentId = objectId;
}

QString ConfigurationElement::GetName() const
{
  return name;
}

void ConfigurationElement::SetName(const QString& name)
{
  this->name = name;
}

void ConfigurationElement::SetParentType(short type)
{
  parentType = type;
}

QObject* ConfigurationElement::CreateExecutableExtension(const QString& attributeName)
{
  QString prop;
  QString executable;
  QString contributorName;
  QString className;
  Object::Pointer initData;
  int i = 0;

  if (!attributeName.isEmpty())
  {
    prop = GetAttribute(attributeName);
  }
  else
  {
    // property not specified, try as element value
    prop = GetValue().trimmed();
  }

  if (prop.isEmpty())
  {
    // property not defined, try as a child element
    QList<ConfigurationElement::Pointer> exec = GetChildren(attributeName);
    if (!exec.empty())
    {
      ConfigurationElement::Pointer element = exec[0]; // assumes single definition
      contributorName = element->GetAttribute("plugin");
      className = element->GetAttribute("class");
      QList<ConfigurationElement::Pointer> parms = element->GetChildren("parameter");
      if (!parms.empty())
      {
        QHash<QString,QString> initParms;
        for (i = 0; i < parms.size(); i++)
        {
          QString pname = parms[i]->GetAttribute("name");
          if (!pname.isEmpty())
            initParms.insert(pname, parms[i]->GetAttribute("value"));
        }
        if (!initParms.isEmpty())
          initData = new ObjectStringMap(initParms);
      }
    }
    else
    {
      // specified name is not a simple attribute nor child element
      ThrowException(QString("Executable extension definition for \"%1\" not found.").arg(attributeName));
    }
  }
  else
  {
    // simple property or element value, parse it into its components
    i = prop.indexOf(QRegExp("[^:]:[^:]"));
    if (i != -1)
    {
      executable = prop.left(i+1).trimmed();
      initData = new ObjectString(prop.mid(i + 2).trimmed());
    }
    else
    {
      executable = prop;
    }

    i = executable.indexOf('/');
    if (i != -1)
    {
      contributorName = executable.left(i).trimmed();
      className = executable.mid(i + 1).trimmed();
    }
    else
    {
      className = executable;
    }
  }

  // create a new instance
  RegistryContributor::Pointer defaultContributor = registry->GetObjectManager()->GetContributor(contributorId);
  QObject* result = registry->CreateExecutableExtension(defaultContributor, className, contributorName);

  // Check if we have extension adapter and initialize;
  // Make the call even if the initialization string is null
  try
  {
    // We need to take into account both "old" and "new" style executable extensions
    if (IExecutableExtension* execExt = qobject_cast<IExecutableExtension*>(result))
    {
      ConfigurationElementHandle::Pointer confElementHandle(new ConfigurationElementHandle(
                                                              registry->GetObjectManager(), GetObjectId()));
      execExt->SetInitializationData(confElementHandle, attributeName, initData);
    }
  }
  catch (const CoreException& ce)
  {
    // user code threw exception
    throw ce;
  }
  catch (const ctkException& te)
  {
    // user code caused exception
    ThrowException(QString("Plug-in \"%1\" was unable to execute setInitializationData on an instance of \"%2\".")
                   .arg(GetContributor()->GetName()).arg(className), te);
  }

  // Deal with executable extension factories.
  if (IExecutableExtensionFactory* execExtFactory = qobject_cast<IExecutableExtensionFactory*>(result))
  {
    result = execExtFactory->Create();
  }

  return result;
}

QString ConfigurationElement::GetAttribute(const QString& attrName, const QLocale& /*locale*/) const
{
  registry->LogMultiLangError();
  return GetAttribute(attrName);
}

QString ConfigurationElement::GetValue(const QLocale& /*locale*/) const
{
  registry->LogMultiLangError();
  return GetValue();
}

QString ConfigurationElement::GetAttribute(const QString& attrName) const
{
  return GetAttributeAsIs(attrName);
}

QList<ConfigurationElement::Pointer> ConfigurationElement::GetChildren(const QString& childrenName) const
{
  QList<ConfigurationElement::Pointer> result;
  if (GetRawChildren().empty())
    return result;

  RegistryObjectManager::Pointer objectManager = registry->GetObjectManager();
  for (int i = 0; i < children.size(); i++)
  {
    ConfigurationElement::Pointer toTest = objectManager->GetObject(
          children[i], NoExtraData() ? RegistryObjectManager::CONFIGURATION_ELEMENT : RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT).Cast<ConfigurationElement>();
    if (toTest->name == childrenName)
    {
      result.push_back(toTest);
    }
  }
  return result;
}

SmartPointer<IContributor> ConfigurationElement::GetContributor() const
{
  return registry->GetObjectManager()->GetContributor(contributorId);
}

}
