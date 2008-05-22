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

#include "cherryPropertyTesterDescriptor.h"

#include "org.opencherry.osgi/cherryPlatform.h"
#include "org.opencherry.osgi/cherryPlatformException.h"

#include "Poco/String.h"

namespace cherry
{

const std::string PropertyTesterDescriptor::PROPERTIES= "properties"; //$NON-NLS-1$
const std::string PropertyTesterDescriptor::NAMESPACE= "namespace"; //$NON-NLS-1$
const std::string PropertyTesterDescriptor::CLASS= "class"; //$NON-NLS-1$

PropertyTesterDescriptor::PropertyTesterDescriptor(IConfigurationElement::Pointer element)
 : fConfigElement(element)
{
  fNamespace = "";
  fConfigElement->GetAttribute(NAMESPACE, fNamespace);
  if (fNamespace.size() == 0)
  {
    throw new CoreException("No namespace");
  }
  std::string buffer(",");
  std::string properties = "";
  fConfigElement->GetAttribute(PROPERTIES, properties);
  if (properties.size() == 0)
  {
    throw new CoreException("No properties");
  }
  Poco::translateInPlace(buffer, "\r\n\t ", "");
//  std::string::iterator iter;
//  for (iter = properties.begin(); iter != properties.end(); ++iter)
//  {
//    char ch= properties.charAt(i);
//    if (!Character.isWhitespace(ch))
//    buffer.append(ch);
//  }
//  buffer.append(',');
  fProperties = buffer;
}

PropertyTesterDescriptor::PropertyTesterDescriptor(IConfigurationElement::Pointer element, const std::string& namespaze, const std::string& properties)
 : fConfigElement(element), fNamespace(namespaze), fProperties(properties)
{
  
}

const std::string&
PropertyTesterDescriptor::GetProperties()
{
  return fProperties;
}

const std::string&
PropertyTesterDescriptor::GetNamespace()
{
  return fNamespace;
}

IConfigurationElement::Pointer
PropertyTesterDescriptor::GetExtensionElement()
{
  return fConfigElement;
}

bool
PropertyTesterDescriptor::Handles(const std::string& namespaze, const std::string& property)
{
  return fNamespace == namespaze && fProperties.find("," + property + ",") != std::string::npos;
}

bool
PropertyTesterDescriptor::IsInstantiated()
{
  return false;
}

bool
PropertyTesterDescriptor::IsDeclaringPluginActive()
{
  IBundle::Pointer fBundle= Platform::GetBundle(fConfigElement->GetContributor());
  return fBundle->IsActive();
}

IPropertyTester*
PropertyTesterDescriptor::Instantiate()
{
  return fConfigElement->CreateExecutableExtension<IPropertyTester>("cherryIPropertyTester", CLASS);
}

bool
PropertyTesterDescriptor::Test(ExpressionVariable::Pointer receiver, const std::string& method, std::vector<ExpressionVariable::Pointer>& args, ExpressionVariable::Pointer expectedValue)
{
  poco_bugcheck_msg("Method should never be called");
  return false;
}

}
