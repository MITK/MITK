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
          ->CreateExecutableExtension<IInputDevice>(
            mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_CLASS,
            IInputDevice::GetManifestName());
    }
  }
  return this->m_InputDevice;
}

std::string mitk::InputDeviceDescriptor::GetID() const 
{
  std::string idOfExtensionPoint;
  this->m_InputDeviceExtensionPoint->GetAttribute(mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_ID,idOfExtensionPoint);
  return idOfExtensionPoint;
}

std::string mitk::InputDeviceDescriptor::GetDescription() const
{
  std::vector<berry::IConfigurationElement::Pointer> 
    descriptions(this->m_InputDeviceExtensionPoint->GetChildren(mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_DESCRIPTION));

  if(!descriptions.empty())
  {
    return descriptions[0]->GetValue();
  }
  return "";
}

std::string mitk::InputDeviceDescriptor::GetName() const
{
  std::string name;
  this->m_InputDeviceExtensionPoint->GetAttribute(mitk::CoreExtConstants::INPUTDEVICE_XMLATTRIBUTE_NAME,name);
  return name;
}

bool mitk::InputDeviceDescriptor::operator==(const Object* object) const
{
  if (const InputDeviceDescriptor* other = dynamic_cast<const InputDeviceDescriptor*>(object))
  {
    return this->GetID() == other->GetID();
  }
  return false;
}

