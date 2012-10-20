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

#ifndef BERRYPROPERTYTESTERDESCRIPTOR_H_
#define BERRYPROPERTYTESTERDESCRIPTOR_H_

#include "berryIPropertyTester.h"

#include "Poco/Any.h"

namespace berry
{

struct IConfigurationElement;

class PropertyTesterDescriptor : public IPropertyTester {

public:
  berryObjectMacro(PropertyTesterDescriptor);

private:

  const SmartPointer<IConfigurationElement> fConfigElement;
  QString fNamespace;
  QString fProperties;

  static const QString PROPERTIES;
  static const QString NAMESPACE;
  static const QString CLASS;

public:

  PropertyTesterDescriptor(const SmartPointer<IConfigurationElement>& element);

  PropertyTesterDescriptor(const SmartPointer<IConfigurationElement>& element,
                           const QString& namespaze, const QString& properties);

  QString GetProperties();

  QString GetNamespace();

  SmartPointer<IConfigurationElement> GetExtensionElement();

  bool Handles(const QString& namespaze, const QString& property);

  bool IsInstantiated();

  bool IsDeclaringPluginActive();

  IPropertyTester* Instantiate();

  bool Test(Object::ConstPointer receiver, const QString& method,
            const QList<Object::Pointer>& args, Object::Pointer expectedValue);
};

}  // namespace berry

#endif /*BERRYPROPERTYTESTERDESCRIPTOR_H_*/
