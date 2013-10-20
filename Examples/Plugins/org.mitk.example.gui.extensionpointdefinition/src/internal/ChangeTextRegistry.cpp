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

#include <berryPlatform.h>
#include <berryIExtensionRegistry.h>
#include <berryIConfigurationElement.h>
#include "ChangeTextRegistry.h"
#include "ExtensionPointDefinitionConstants.h"

ChangeTextRegistry::ChangeTextRegistry()
{
  //initialize the registry by copying all available extension points into a local variable
  berry::IExtensionRegistry* extensionRegistry = berry::Platform::GetExtensionRegistry();
  QList<berry::IConfigurationElement::Pointer> allExtensionsChangeTexts
    = extensionRegistry->GetConfigurationElementsFor(ExtensionPointDefinitionConstants::CHANGETEXT_XP_NAME);

  foreach(const berry::IConfigurationElement::Pointer& it, allExtensionsChangeTexts)
  {
    QString id = it->GetAttribute(ExtensionPointDefinitionConstants::CHANGETEXT_XMLATTRIBUTE_ID);
    if(id.isEmpty() || this->m_ListRegisteredTexts.contains(id))
    {
      BERRY_WARN << "The ChangeText ID: " << id << " is already registered.";
    }
    else
    {
      ChangeTextDescriptor::Pointer tmp(new ChangeTextDescriptor(it));
      m_ListRegisteredTexts.insert(id, tmp);
    }
  }
}

ChangeTextRegistry::~ChangeTextRegistry()
{
}

ChangeTextDescriptor::Pointer ChangeTextRegistry::Find(const QString &id) const
{
  return this->m_ListRegisteredTexts.value(id);
}

QList<ChangeTextDescriptor::Pointer> ChangeTextRegistry::GetChangeTexts() const
{
  return m_ListRegisteredTexts.values();
}
