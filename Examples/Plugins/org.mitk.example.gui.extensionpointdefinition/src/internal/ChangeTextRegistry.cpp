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
#include <berryIExtensionPointService.h>
#include <berryIConfigurationElement.h>
#include "ChangeTextRegistry.h"
#include "ExtensionPointDefinitionConstants.h"

ChangeTextRegistry::ChangeTextRegistry()
{
  //initialize the registry by copying all available extension points into a local variable
  berry::IExtensionPointService::Pointer extensionPointService = berry::Platform::GetExtensionPointService();
  std::vector<berry::IConfigurationElement::Pointer> allExtensionsChangeTexts
    = extensionPointService->GetConfigurationElementsFor(ExtensionPointDefinitionConstants::CHANGETEXT_XP_NAME);

  for(std::vector<berry::IConfigurationElement::Pointer>::const_iterator it = allExtensionsChangeTexts.begin();
    it != allExtensionsChangeTexts.end();++it)
  {
    ChangeTextDescriptor::Pointer temp(new ChangeTextDescriptor(*it));

    if(!this->m_ListRegisteredTexts.contains(temp->GetID()))
    {
      m_ListRegisteredTexts.insert(temp->GetID(),temp);
    }
    else
    {
      BERRY_WARN << "The ChangeText ID: " << qPrintable(temp->GetID()) << " is already registered.";
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
