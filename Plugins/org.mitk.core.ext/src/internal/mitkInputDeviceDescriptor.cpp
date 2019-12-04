/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkInputDeviceDescriptor.h"
#include "mitkCoreExtConstants.h"


mitk::InputDeviceDescriptor::InputDeviceDescriptor(berry::IConfigurationElement::Pointer inputDeviceExtensionPoint)
: IInputDeviceDescriptor(), m_InputDeviceExtensionPoint(inputDeviceExtensionPoint)
{
}

mitk::InputDeviceDescriptor::~InputDeviceDescriptor()
{
}

mitk::IInputDevice::Pointer mitk::InputDeviceDescriptor::CreateInputDevice()
{
  if(this->m_InputDevice == 0)
  {
    // "class" refers to xml attribute in a xml tag
    this->m_InputDevice = this->m_InputDeviceExtensionPoint
      ->CreateExecutableExtension<mitk::IInputDevice>(mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_CLASS);
    if (this->m_InputDevice == 0)
    {
      // support legacy BlueBerry extensions
      this->m_InputDevice = this->m_InputDeviceExtensionPoint
          ->CreateExecutableExtension<IInputDevice>(mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_CLASS);
    }
  }
  return this->m_InputDevice;
}

QString mitk::InputDeviceDescriptor::GetID() const
{
  return this->m_InputDeviceExtensionPoint->GetAttribute(mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_ID);
}

QString mitk::InputDeviceDescriptor::GetDescription() const
{
  QList<berry::IConfigurationElement::Pointer>
    descriptions(this->m_InputDeviceExtensionPoint->GetChildren(mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_DESCRIPTION));

  if(!descriptions.empty())
  {
    return descriptions[0]->GetValue();
  }
  return QString();
}

QString mitk::InputDeviceDescriptor::GetName() const
{
  return this->m_InputDeviceExtensionPoint->GetAttribute(mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_NAME);
}

bool mitk::InputDeviceDescriptor::operator==(const Object* object) const
{
  if (const InputDeviceDescriptor* other = dynamic_cast<const InputDeviceDescriptor*>(object))
  {
    return this->GetID() == other->GetID();
  }
  return false;
}

