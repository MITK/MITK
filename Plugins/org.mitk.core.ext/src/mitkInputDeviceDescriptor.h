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

#ifndef MITKINPUTDEVICEDESCRIPTOR_H_
#define MITKINPUTDEVICEDESCRIPTOR_H_

#include <berryIConfigurationElement.h>

#include <string>

#include "mitkIInputDeviceDescriptor.h"
#include "mitkIInputDevice.h"

namespace mitk
{
  /**
  * Documentation in the interface.
  *
  * @see mitk::IInputDeviceDescriptor
  * @ingroup org_mitk_core_ext
  */
  class InputDeviceDescriptor : public IInputDeviceDescriptor
  {

  public:

    /**
    * Initialize the Input Device Descriptor with the given extension point.
    *
    * @param inputDeviceExtensionPoint
    *        element, that refers to a extension point (type, id, name, class)
    */
    InputDeviceDescriptor(berry::IConfigurationElement::Pointer inputDeviceExtensionPoint);

    /**
    * Default destructor
    */
    ~InputDeviceDescriptor();

    /**
    * @see mitk::IInputDeviceDescriptor::CreateInputDevice()
    */
    mitk::IInputDevice::Pointer CreateInputDevice();

    /**
    * @see mitk::IInputDeviceDescriptor::GetDescription()
    */
    std::string GetDescription() const;

    /**
    * @see mitk::IInputDeviceDescriptor::GetID()
    */
    std::string GetID() const;

    /**
    * @see mitk::IInputDeviceDescriptor::GetName()
    */
    std::string GetName() const;

    /**
    * @see mitk::IInputDeviceDescriptor::operator==(const Object* object)
    */
    bool operator==(const Object* object) const;

  private:

    // IConfigurationElements are used to access xml files (here: plugin.xml)
    berry::IConfigurationElement::Pointer m_InputDeviceExtensionPoint;
    mitk::IInputDevice::Pointer m_InputDevice;

  }; // end class
} // end namespace

#endif /*MITKINPUTDEVICEDESCRIPTOR_H_*/
