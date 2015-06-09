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

  QString GetAttribute(const QString& propertyName) const override;

  QString GetAttribute(const QString& attrName, const QLocale& locale) const;

  QList<QString> GetAttributeNames() const override;

  QList<IConfigurationElement::Pointer> GetChildren() const override;

  QObject* CreateExecutableExtension(const QString& propertyName) const override;

  QString GetAttributeAsIs(const QString& name) const;

  QList<IConfigurationElement::Pointer> GetChildren(const QString& name) const override;

  SmartPointer<IExtension> GetDeclaringExtension() const override;

  QString GetName() const override;

  SmartPointer<Object> GetParent() const override;

  QString GetValue() const override;

  QString GetValue(const QLocale& locale) const override;

  QString GetValueAsIs() const;

  SmartPointer<RegistryObject> GetObject() const override;

  QString GetNamespaceIdentifier() const override;

  SmartPointer<IContributor> GetContributor() const override;

  bool IsValid() const override;

protected:

  virtual SmartPointer<ConfigurationElement> GetConfigurationElement() const;

  bool ShouldPersist() const;
};

}

#endif // BERRYCONFIGURATIONELEMENTHANDLE_H
