/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "ChangeTextDescriptor.h"
#include "ExtensionPointDefinitionConstants.h"

#include "berryPlatformException.h"

ChangeTextDescriptor::ChangeTextDescriptor(berry::IConfigurationElement::Pointer changeTextExtensionPoint)
  : m_ChangeTextExtensionPoint(changeTextExtensionPoint)
{
  std::string id;
  std::string name;

  // Check if the "id" and "name" attributes are available
  if (!this->m_ChangeTextExtensionPoint->GetAttribute(ExtensionPointDefinitionConstants::CHANGETEXT_XMLATTRIBUTE_ID, id) ||
      !this->m_ChangeTextExtensionPoint->GetAttribute(ExtensionPointDefinitionConstants::CHANGETEXT_XMLATTRIBUTE_NAME, name))
  {
    throw berry::CoreException("Invalid changetext configuration element (missing id or name) from: " +
                               m_ChangeTextExtensionPoint->GetContributor());
  }

  this->m_Id = QString::fromStdString(id);
  this->m_Name = QString::fromStdString(name);

  // Get the optional "description" child element
  std::vector<berry::IConfigurationElement::Pointer> descriptions(
        this->m_ChangeTextExtensionPoint->GetChildren(ExtensionPointDefinitionConstants::CHANGETEXT_CHILD_DESCRIPTION));

  if(!descriptions.empty())
  {
    this->m_Description = QString::fromStdString(descriptions[0]->GetValue());
  }
}

ChangeTextDescriptor::~ChangeTextDescriptor()
{
}

IChangeText::Pointer ChangeTextDescriptor::CreateChangeText()
{
  if(this->m_ChangeText == 0)
  {
    // "class" refers to xml attribute in a xml tag
    this->m_ChangeText = this->m_ChangeTextExtensionPoint
      ->CreateExecutableExtension<IChangeText>(ExtensionPointDefinitionConstants::CHANGETEXT_XMLATTRIBUTE_CLASS);
  }
  return this->m_ChangeText;
}

QString ChangeTextDescriptor::GetID() const
{
  return this->m_Id;
}

QString ChangeTextDescriptor::GetDescription() const
{
  return this->m_Description;
}

QString ChangeTextDescriptor::GetName() const
{
  return this->m_Name;
}

bool ChangeTextDescriptor::operator==(const Object* object) const
{
  if (const ChangeTextDescriptor* other = dynamic_cast<const ChangeTextDescriptor*>(object))
  {
    return this->GetID() == other->GetID();
  }
  return false;
}
