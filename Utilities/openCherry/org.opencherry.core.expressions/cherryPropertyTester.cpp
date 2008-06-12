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

#include "cherryPropertyTester.h"

#include "cherryPlatform.h"

namespace cherry
{

void PropertyTester::InternalInitialize(PropertyTesterDescriptor* descriptor)
{
  fProperties= descriptor->GetProperties();
  fNamespace= descriptor->GetNamespace();
  fConfigElement= descriptor->GetExtensionElement();
}

PropertyTesterDescriptor* PropertyTester::InternalCreateDescriptor()
{
  return new PropertyTesterDescriptor(fConfigElement, fNamespace, fProperties);
}

bool PropertyTester::Handles(const std::string& namespaze,
    const std::string& property)
{
  return fNamespace == namespaze && fProperties.find("," + property
      + ",") != std::string::npos; 
}

bool PropertyTester::IsInstantiated()
{
  return true;
}

bool PropertyTester::IsDeclaringPluginActive()
{
  IBundle::Pointer bundle= Platform::GetBundle(fConfigElement->GetContributor());
  return bundle->IsActive();
}

IPropertyTester* PropertyTester::Instantiate()
{
  return this;
}


}  // namespace cherry
