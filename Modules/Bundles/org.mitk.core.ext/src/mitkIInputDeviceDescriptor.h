/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date: 2010-01-16 19:57:43 +0100 (Sat, 16 Jan 2010) $
Version:   $Revision: 21070 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKIINPUTDEVICEDESCRIPTOR_H_
#define MITKIINPUTDEVICEDESCRIPTOR_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <string>
#include <vector>

#include "mitkIInputDevice.h"

namespace mitk
{
  /**
  * This is an input device descriptor. It provides a "description" of a given
  * input device, so that the input device can later be constructed and registered.
  * <p>
  * The input device registry provides facilities to map from an extension
  * to a IInputDeviceDescriptor.
  * </p>
  * <p>
  * This interface is not intended to be implemented by clients.
  * </p>
  *
  * @see mitk::IInputDeviceRegistry
  * @ingroup org_mitk_core_ext
  */
  struct IInputDeviceDescriptor : public berry::Object
  {

    berryInterfaceMacro(IInputDeviceDescriptor, mitk);

    /**
    * Creates an instance of an input device defined in the descriptor.
    *
    * @return the input device
    */
    virtual IInputDevice::Pointer CreateInputDevice() = 0;

    /**
    * Returns the description of this input device.
    *
    * @return the description
    */
    virtual std::string GetDescription() const = 0;

    /**
    * Returns the id of this input device.
    *
    * @return the id
    */
    virtual std::string GetID() const = 0;

    /**
    * Returns the name of this input device.
    *
    * @return the name 
    */
    virtual std::string GetName() const = 0;

    // /**
    // * Returns the descriptor for the icon to show for this view.
    // */
    //virtual SmartPointer<ImageDescriptor> GetImageDescriptor() const = 0;

    /**
    * Equals this class with the given parameter.
    *
    * @param object 
    *			the object for the equation
    * @return true, if the objects are equal :: false, if they differ in any way
    */
    virtual bool operator==(const Object* object) const = 0;

  }; // end struct IInputDeviceDescriptor
} //end namespace mitk

#endif /*MITKIINPUTDEVICEDESCRIPTOR_H_*/
