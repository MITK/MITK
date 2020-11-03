/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <berryPlatform.h>
#include <berryIExtensionRegistry.h>
#include <berryIConfigurationElement.h>

#include <mitkCoreExtConstants.h>

#include "mitkInputDeviceRegistry.h"
#include "mitkInputDeviceDescriptor.h"


mitk::InputDeviceRegistry::InputDeviceRegistry()
{
  //initialize the registry by copying all available extension points into a local variable
  berry::IExtensionRegistry* extensionPointService = berry::Platform::GetExtensionRegistry();
  QList<berry::IConfigurationElement::Pointer> allExtensionsInputDevices
    = extensionPointService->GetConfigurationElementsFor(mitk::CoreExtConstants::INPUTDEVICE_EXTENSION_NAME);

  for(QList<berry::IConfigurationElement::Pointer>::const_iterator it = allExtensionsInputDevices.begin();
    it != allExtensionsInputDevices.end();++it)
  {
    InputDeviceDescriptorPtr temp(new mitk::InputDeviceDescriptor(*it));

    // The equation with the end means, that if there is no such element and
    // the pointer will be at end (not the last element, actually after it)
    if(this->m_ListRegisteredDevices.find(temp->GetID()) == this->m_ListRegisteredDevices.end())
    {
      m_ListRegisteredDevices.insert(temp->GetID(), temp);
    }
    else
    {
      throw ctkRuntimeException("The Input Device ID: " + temp->GetID() + " is already registered.");
    }
  }
}

mitk::InputDeviceRegistry::~InputDeviceRegistry()
{
}

mitk::InputDeviceRegistry::InputDeviceDescriptorPtr mitk::InputDeviceRegistry::Find(const QString &id) const
{
  QHash<QString,InputDeviceDescriptorPtr>::ConstIterator result = this->m_ListRegisteredDevices.find(id);

  // first = key, second = element or vice versa, if inserted different in the hash map
  if(result != this->m_ListRegisteredDevices.end()) return result.value();

  return InputDeviceDescriptorPtr(nullptr);
}

QList<mitk::InputDeviceRegistry::InputDeviceDescriptorPtr> mitk::InputDeviceRegistry::GetInputDevices() const
{
  return m_ListRegisteredDevices.values();
}
