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

#include "berryPropertyTesterDescriptor.h"

#include "berryPlatform.h"
#include "berryPlatformException.h"

#include <berryIConfigurationElement.h>

#include "Poco/String.h"

namespace berry
{

const QString PropertyTesterDescriptor::PROPERTIES= "properties";
const QString PropertyTesterDescriptor::NAMESPACE= "namespace";
const QString PropertyTesterDescriptor::CLASS= "class";

PropertyTesterDescriptor::PropertyTesterDescriptor(const IConfigurationElement::Pointer& element)
 : fConfigElement(element)
{
  fNamespace = fConfigElement->GetAttribute(NAMESPACE);
  if (fNamespace.isNull())
  {
    throw CoreException("No namespace");
  }
  QString buffer(",");
  QString properties = fConfigElement->GetAttribute(PROPERTIES);
  if (properties.isNull())
  {
    throw CoreException("No properties");
  }
  foreach(QChar ch, properties)
  {
    if (!ch.isSpace())
    {
      buffer.append(ch);
    }
  }
  buffer.append(',');
  fProperties = buffer;
}

PropertyTesterDescriptor::PropertyTesterDescriptor(const IConfigurationElement::Pointer& element,
                                                   const QString& namespaze, const QString& properties)
 : fConfigElement(element), fNamespace(namespaze), fProperties(properties)
{

}

QString
PropertyTesterDescriptor::GetProperties()
{
  return fProperties;
}

QString PropertyTesterDescriptor::GetNamespace()
{
  return fNamespace;
}

IConfigurationElement::Pointer
PropertyTesterDescriptor::GetExtensionElement()
{
  return fConfigElement;
}

bool
PropertyTesterDescriptor::Handles(const QString &namespaze, const QString &property)
{
  return fNamespace == namespaze && fProperties.contains("," + property + ",");
}

bool
PropertyTesterDescriptor::IsInstantiated()
{
  return false;
}

bool
PropertyTesterDescriptor::IsDeclaringPluginActive()
{
  QSharedPointer<ctkPlugin> plugin = Platform::GetPlugin(fConfigElement->GetContributor());
  return plugin->getState() == ctkPlugin::ACTIVE;
}

IPropertyTester*
PropertyTesterDescriptor::Instantiate()
{
  return fConfigElement->CreateExecutableExtension<IPropertyTester>(CLASS);
}

bool
PropertyTesterDescriptor::Test(Object::Pointer  /*receiver*/, const QString &  /*method*/,
                               const QList<Object::Pointer> &  /*args*/, Object::Pointer  /*expectedValue*/)
{
  poco_bugcheck_msg("Method should never be called");
  return false;
}

}
