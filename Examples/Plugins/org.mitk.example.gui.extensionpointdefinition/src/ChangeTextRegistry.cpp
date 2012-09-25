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
#include <ChangeTextRegistry.h>
#include "ExtensionPointDefinitionConstants.h"

ChangeTextRegistry::ChangeTextRegistry()
{
  //initialize the registry by copying all available extension points into a local variable
  berry::IExtensionPointService::Pointer extensionPointService = berry::Platform::GetExtensionPointService();
  std::vector<berry::IConfigurationElement::Pointer> allExtensionsChangeTexts 
    = extensionPointService->GetConfigurationElementsFor(ExtensionPointDefinitionConstants::CHANGETEXT_EXTENSION_NAME);

  for(std::vector<berry::IConfigurationElement::Pointer>::const_iterator it = allExtensionsChangeTexts.begin();
    it != allExtensionsChangeTexts.end();++it)
  {
    ChangeTextDescriptorPtr temp(new ChangeTextDescriptor(*it));

    // The equation with the end means, that if there is no such element and 
    // the pointer will be at end (not the last element, actually after it)
    if(this->m_ListRegisteredTexts.find(temp->GetID()) == this->m_ListRegisteredTexts.end())
    {
      m_ListRegisteredTexts.insert(std::make_pair<std::string, ChangeTextDescriptorPtr>(temp->GetID(),temp));
    }
    else
    {
      throw std::runtime_error("The ChangeText ID: "+temp->GetID()+" is already registered.");
    }
  }
}

ChangeTextRegistry::~ChangeTextRegistry()
{
}

ChangeTextRegistry::ChangeTextDescriptorPtr ChangeTextRegistry::Find(const std::string &id) const
{
  Poco::HashMap<std::string,ChangeTextDescriptorPtr>::ConstIterator result = this->m_ListRegisteredTexts.find(id);

  // first = key, second = element or vice versa, if inserted different in the hash map
  if(result != this->m_ListRegisteredTexts.end()) return result->second;

  return ChangeTextDescriptorPtr(0);
}

std::vector<ChangeTextRegistry::ChangeTextDescriptorPtr> ChangeTextRegistry::GetChangeTexts() const
{
  std::vector<ChangeTextRegistry::ChangeTextDescriptorPtr> temp;
  for(Poco::HashMap<std::string, ChangeTextDescriptorPtr>::ConstIterator it = m_ListRegisteredTexts.begin(); 
    it != m_ListRegisteredTexts.end();++it)
  {
    // first = key, second = element or vice versa, if inserted different in the hash map
    temp.push_back(it->second);
  }
  return temp;
}

