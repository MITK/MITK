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

#include <mitkCoreExtConstants.h>

#include "mitkInputDeviceRegistry.h"
#include "mitkInputDeviceDescriptor.h"


mitk::InputDeviceRegistry::InputDeviceRegistry()
{
  //initialize the registry by copying all available extension points into a local variable
  berry::IExtensionPointService* extensionPointService = berry::Platform::GetExtensionPointService();
  std::vector<berry::IConfigurationElement::Pointer> allExtensionsInputDevices
    = extensionPointService->GetConfigurationElementsFor(mitk::CoreExtConstants::INPUTDEVICE_EXTENSION_NAME);

  for(std::vector<berry::IConfigurationElement::Pointer>::const_iterator it = allExtensionsInputDevices.begin();
    it != allExtensionsInputDevices.end();++it)
  {
    InputDeviceDescriptorPtr temp(new mitk::InputDeviceDescriptor(*it));

    // The equation with the end means, that if there is no such element and
    // the pointer will be at end (not the last element, actually after it)
    if(this->m_ListRegisteredDevices.find(temp->GetID()) == this->m_ListRegisteredDevices.end())
    {
      m_ListRegisteredDevices.insert(std::make_pair(temp->GetID(),temp));
    }
    else
    {
      throw std::runtime_error("The Input Device ID: "+temp->GetID()+" is already registered.");
    }
  }
}

mitk::InputDeviceRegistry::~InputDeviceRegistry()
{
}

mitk::InputDeviceRegistry::InputDeviceDescriptorPtr mitk::InputDeviceRegistry::Find(const std::string &id) const
{
  Poco::HashMap<std::string,InputDeviceDescriptorPtr>::ConstIterator result = this->m_ListRegisteredDevices.find(id);

  // first = key, second = element or vice versa, if inserted different in the hash map
  if(result != this->m_ListRegisteredDevices.end()) return result->second;

  return InputDeviceDescriptorPtr(0);
}

std::vector<mitk::InputDeviceRegistry::InputDeviceDescriptorPtr> mitk::InputDeviceRegistry::GetInputDevices() const
{
  std::vector<mitk::InputDeviceRegistry::InputDeviceDescriptorPtr> temp;
  for(Poco::HashMap<std::string, InputDeviceDescriptorPtr>::ConstIterator it = m_ListRegisteredDevices.begin();
    it != m_ListRegisteredDevices.end();++it)
  {
    // first = key, second = element or vice versa, if inserted different in the hash map
    temp.push_back(it->second);
  }
  return temp;
}
