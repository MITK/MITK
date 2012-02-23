/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include <berryPlatform.h>
#include <berryIExtensionPointService.h>
#include <berryIConfigurationElement.h>
#include <mitkInputDeviceRegistry.h>
#include <mitkInputDeviceDescriptor.h>
#include "mitkCoreExtConstants.h"

mitk::InputDeviceRegistry::InputDeviceRegistry()
{
  //initialize the registry by copying all available extension points into a local variable
  berry::IExtensionPointService::Pointer extensionPointService = berry::Platform::GetExtensionPointService();
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
      m_ListRegisteredDevices.insert(std::make_pair<std::string, InputDeviceDescriptorPtr>(temp->GetID(),temp));
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

bool mitk::InputDeviceRegistry::IsA(const std::type_info& type) const
{
  std::string name(GetType().name());
  return name == type.name() || berry::Service::IsA(type);
}

const std::type_info& mitk::InputDeviceRegistry::GetType() const
{
  return typeid(IInputDeviceRegistry);
}

