/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "ChangeTextDescriptor.h"
#include "ExtensionPointDefinitionConstants.h"

#include "berryIContributor.h"

ChangeTextDescriptor::ChangeTextDescriptor(berry::IConfigurationElement::Pointer changeTextExtensionPoint)
  : m_ChangeTextExtensionPoint(changeTextExtensionPoint)
{
  this->m_Id =
    this->m_ChangeTextExtensionPoint->GetAttribute(ExtensionPointDefinitionConstants::CHANGETEXT_XMLATTRIBUTE_ID);
  this->m_Name =
    this->m_ChangeTextExtensionPoint->GetAttribute(ExtensionPointDefinitionConstants::CHANGETEXT_XMLATTRIBUTE_NAME);

  // Check if the "id" and "name" attributes are available
  if (this->m_Id.isNull() || this->m_Name.isNull())
  {
    throw ctkRuntimeException("Invalid changetext configuration element (missing id or name) from: " +
                              m_ChangeTextExtensionPoint->GetContributor()->GetName());
  }

  // Get the optional "description" child element
  QList<berry::IConfigurationElement::Pointer> descriptions(
    this->m_ChangeTextExtensionPoint->GetChildren(ExtensionPointDefinitionConstants::CHANGETEXT_CHILD_DESCRIPTION));

  if (!descriptions.isEmpty())
  {
    this->m_Description = descriptions[0]->GetValue();
  }
}

ChangeTextDescriptor::~ChangeTextDescriptor()
{
}

IChangeText::Pointer ChangeTextDescriptor::CreateChangeText()
{
  if (this->m_ChangeText == 0)
  {
    // "class" refers to xml attribute in a xml tag
    this->m_ChangeText = this->m_ChangeTextExtensionPoint->CreateExecutableExtension<IChangeText>(
      ExtensionPointDefinitionConstants::CHANGETEXT_XMLATTRIBUTE_CLASS);
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

bool ChangeTextDescriptor::operator==(const Object *object) const
{
  if (const ChangeTextDescriptor *other = dynamic_cast<const ChangeTextDescriptor *>(object))
  {
    return this->GetID() == other->GetID();
  }
  return false;
}
