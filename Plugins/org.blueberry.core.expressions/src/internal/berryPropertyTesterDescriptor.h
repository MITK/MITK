/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  bool Handles(const QString& namespaze, const QString& property) override;

  bool IsInstantiated() override;

  bool IsDeclaringPluginActive() override;

  IPropertyTester* Instantiate() override;

  bool Test(Object::ConstPointer receiver, const QString& method,
            const QList<Object::Pointer>& args, Object::Pointer expectedValue) override;
};

}  // namespace berry

#endif /*BERRYPROPERTYTESTERDESCRIPTOR_H_*/
