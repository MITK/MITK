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

#include "ViewTagsDescriptor.h"
#include "ExtensionPointDefinitionConstants.h"

#include "berryPlatformException.h"

ViewTagsDescriptor::ViewTagsDescriptor(QString id)
{
  this->m_Id = id;
  m_Tags.clear();
}
ViewTagsDescriptor::ViewTagsDescriptor(berry::IConfigurationElement::Pointer changeTextExtensionPoint)
  : m_ChangeTextExtensionPoint(changeTextExtensionPoint)
{
  std::string id;
  std::string name;

  // Check if the "id" and "name" attributes are available
  if (!this->m_ChangeTextExtensionPoint->GetAttribute(ExtensionPointDefinitionConstants::VIEWTAGS_XMLATTRIBUTE_ID, id))
  {
    throw berry::CoreException("Invalid changetext configuration element (missing id) from: " +
                               m_ChangeTextExtensionPoint->GetContributor());
  }

  this->m_Id = QString::fromStdString(id);

  // Get the optional "description" child element
  std::vector<berry::IConfigurationElement::Pointer> tags(
        this->m_ChangeTextExtensionPoint->GetChildren(ExtensionPointDefinitionConstants::VIEWTAGS_CHILD_TAG));

  for (int i = 0; i < tags.size(); ++i)
  {
    std::string tmpTag = tags[i]->GetValue();
    m_Tags.push_back(QString::fromStdString(tmpTag));
  }
}

ViewTagsDescriptor::~ViewTagsDescriptor()
{
}

std::vector<QString> ViewTagsDescriptor::GetTags()
{
  return m_Tags;
}

QString ViewTagsDescriptor::GetID() const
{
  return this->m_Id;
}

bool ViewTagsDescriptor::operator==(const Object* object) const
{
  if (const ViewTagsDescriptor* other = dynamic_cast<const ViewTagsDescriptor*>(object))
  {
    return this->GetID() == other->GetID();
  }
  return false;
}