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

#ifndef BERRYCONFIGURATIONELEMENTHANDLE_H
#define BERRYCONFIGURATIONELEMENTHANDLE_H

#include "berryHandle.h"
#include "berryIConfigurationElement.h"

namespace berry {

class ConfigurationElement;

class ConfigurationElementHandle : public Handle, public IConfigurationElement
{

public:

  berryObjectMacro(berry::ConfigurationElementHandle)

  explicit ConfigurationElementHandle(const SmartPointer<const IObjectManager> &objectManager, int id);
  explicit ConfigurationElementHandle(const IObjectManager* objectManager, int id);

  QString GetAttribute(const QString& propertyName) const;

  QString GetAttribute(const QString& attrName, const QLocale& locale) const;

  QList<QString> GetAttributeNames() const;

  QList<IConfigurationElement::Pointer> GetChildren() const;

  QObject* CreateExecutableExtension(const QString& propertyName) const;

  QString GetAttributeAsIs(const QString& name) const;

  QList<IConfigurationElement::Pointer> GetChildren(const QString& name) const;

  SmartPointer<IExtension> GetDeclaringExtension() const;

  QString GetName() const;

  SmartPointer<Object> GetParent() const;

  QString GetValue() const;

  QString GetValue(const QLocale& locale) const;

  QString GetValueAsIs() const;

  SmartPointer<RegistryObject> GetObject() const;

  QString GetNamespaceIdentifier() const;

  SmartPointer<IContributor> GetContributor() const;

  bool IsValid() const;

protected:

  virtual SmartPointer<ConfigurationElement> GetConfigurationElement() const;

  bool ShouldPersist() const;
};

}

#endif // BERRYCONFIGURATIONELEMENTHANDLE_H
