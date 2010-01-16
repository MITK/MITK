/*=========================================================================
 
Program:   BlueBerry Platform
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

#ifndef BERRYPROPERTYTESTERDESCRIPTOR_H_
#define BERRYPROPERTYTESTERDESCRIPTOR_H_

#include "service/berryIConfigurationElement.h"
#include "../berryIPropertyTester.h"

#include "Poco/Any.h"

#include <vector>
#include <string>

namespace berry
{

class PropertyTesterDescriptor : public IPropertyTester {
  
public:
  berryObjectMacro(PropertyTesterDescriptor)
  
private:
  
  IConfigurationElement::Pointer fConfigElement;
  std::string fNamespace;
  std::string fProperties;
  
  static const std::string PROPERTIES;
  static const std::string NAMESPACE;
  static const std::string CLASS;
  
public:
  
  PropertyTesterDescriptor(IConfigurationElement::Pointer element);
  
  PropertyTesterDescriptor(IConfigurationElement::Pointer element, const std::string& namespaze, const std::string& properties);
  
  const std::string& GetProperties();
  
  const std::string& GetNamespace();
  
  IConfigurationElement::Pointer GetExtensionElement();
  
  bool Handles(const std::string& namespaze, const std::string& property);
  
  bool IsInstantiated();
  
  bool IsDeclaringPluginActive();
  
  IPropertyTester* Instantiate();
  
  bool Test(Object::Pointer receiver, const std::string& method, std::vector<Object::Pointer>& args, Object::Pointer expectedValue);
};

}  // namespace berry

#endif /*BERRYPROPERTYTESTERDESCRIPTOR_H_*/
