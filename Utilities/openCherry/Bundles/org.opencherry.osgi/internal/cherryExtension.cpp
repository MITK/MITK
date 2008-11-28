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

#include "cherryExtension.h"

namespace cherry
{

Extension::Extension(const std::string& ns)
 : namespaze(ns)
{

}

bool Extension::operator==(const Extension* em) const
{
  return (id == em->id) && (extensionPoint == em->extensionPoint);
}

std::string Extension::GetNamespace() const
{
  return namespaze;
}

std::string Extension::GetExtensionPointIdentifier() const
{
  return extensionPoint;
}

std::string Extension::GetSimpleIdentifier() const
{
  return id;
}

std::string Extension::GetUniqueIdentifier() const
{
  return id == "" ? "" : this->GetNamespace() + "." + id; //$NON-NLS-1$
}

const std::vector<IConfigurationElement::Pointer> Extension::GetConfigurationElements() const
{
  return elements;
}

std::string Extension::GetLabel() const
{
  return label;
}

void Extension::SetExtensionPointIdentifier(const std::string& value)
{
  extensionPoint = value;
}

void Extension::SetSimpleIdentifier(const std::string& value)
{
  id = value;
}

void Extension::SetSubElements(const std::vector<IConfigurationElement::Pointer>& value)
{
  elements = value;
}

void Extension::SetLabel(const std::string& l)
{
  label = l;
}

bool Extension::operator<(const IExtension* e2) const
{
  return this->GetNamespace() < e2->GetNamespace();
}

} // namespace cherry
