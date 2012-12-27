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

#include "berryExtension.h"

namespace berry
{

Extension::Extension(const std::string& ns)
 : namespaze(ns)
{

}

bool Extension::operator==(const Object* em) const
{
  if (const Extension* other = dynamic_cast<const Extension*>(em))
    return (id == other->id) && (extensionPoint == other->extensionPoint);

  return false;
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

} // namespace berry
