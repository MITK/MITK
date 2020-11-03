/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPropertyTesterDescriptor.h"

#include "berryPlatform.h"
#include "berryCoreException.h"
#include "berryStatus.h"
#include "berryExpressionPlugin.h"

#include <berryIConfigurationElement.h>
#include <berryIContributor.h>

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
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, ExpressionPlugin::GetPluginId(),
                                       IStatus::ERROR_TYPE,
                                       "The mandatory attribute namespace is missing. Tester has been disabled.",
                                       BERRY_STATUS_LOC));
    throw CoreException(status);
  }
  QString buffer(",");
  QString properties = fConfigElement->GetAttribute(PROPERTIES);
  if (properties.isNull())
  {
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, ExpressionPlugin::GetPluginId(),
                                       IStatus::ERROR_TYPE,
                                       "The mandatory attribute properties is missing. Tester has been disabled.",
                                       BERRY_STATUS_LOC));
    throw CoreException(status);
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
  QSharedPointer<ctkPlugin> plugin = Platform::GetPlugin(fConfigElement->GetContributor()->GetName());
  return plugin->getState() == ctkPlugin::ACTIVE;
}

IPropertyTester*
PropertyTesterDescriptor::Instantiate()
{
  return fConfigElement->CreateExecutableExtension<IPropertyTester>(CLASS);
}

bool
PropertyTesterDescriptor::Test(Object::ConstPointer  /*receiver*/, const QString &  /*method*/,
                               const QList<Object::Pointer> &  /*args*/, Object::Pointer  /*expectedValue*/)
{
  poco_bugcheck_msg("Method should never be called");
  return false;
}

}
