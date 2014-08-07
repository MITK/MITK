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
#include "ViewTagsRegistry.h"
#include "ExtensionPointDefinitionConstants.h"

ViewTagsRegistry::ViewTagsRegistry()
{
  //initialize the registry by copying all available extension points into a local variable
  berry::IExtensionPointService::Pointer extensionPointService = berry::Platform::GetExtensionPointService();
  std::vector<berry::IConfigurationElement::Pointer> allExtensionsChangeTexts
    = extensionPointService->GetConfigurationElementsFor(ExtensionPointDefinitionConstants::VIEWTAGS_XP_NAME);

  for(std::vector<berry::IConfigurationElement::Pointer>::const_iterator it = allExtensionsChangeTexts.begin();
    it != allExtensionsChangeTexts.end();++it)
  {
    ViewTagsDescriptor::Pointer temp(new ViewTagsDescriptor(*it));

    if(!this->m_ListRegisteredViewTags.contains(temp->GetID()))
    {
      m_ListRegisteredViewTags.insert(temp->GetID(),temp);
    }
    else
    {
      BERRY_WARN << "The ChangeText ID: " << qPrintable(temp->GetID()) << " is already registered.";
    }
  }
}

ViewTagsRegistry::~ViewTagsRegistry()
{
}

ViewTagsDescriptor::Pointer ViewTagsRegistry::Find(const std::string &id) const
{
  return this->Find(QString::fromStdString(id));
}
ViewTagsDescriptor::Pointer ViewTagsRegistry::Find(const QString &id) const
{
  if (this->m_ListRegisteredViewTags.contains(id))
  {
    return this->m_ListRegisteredViewTags.value(id);
  }
  else
  {
    ViewTagsDescriptor::Pointer tmp(new ViewTagsDescriptor(id));
    return tmp;
  }
}

QList<ViewTagsDescriptor::Pointer> ViewTagsRegistry::GetViewTags() const
{
  return m_ListRegisteredViewTags.values();
}