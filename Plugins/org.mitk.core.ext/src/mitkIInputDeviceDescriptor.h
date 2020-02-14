/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIINPUTDEVICEDESCRIPTOR_H_
#define MITKIINPUTDEVICEDESCRIPTOR_H_

#include <berryObject.h>
#include <berryMacros.h>

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

    berryObjectMacro(mitk::IInputDeviceDescriptor);

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
    virtual QString GetDescription() const = 0;

    /**
    * Returns the id of this input device.
    *
    * @return the id
    */
    virtual QString GetID() const = 0;

    /**
    * Returns the name of this input device.
    *
    * @return the name
    */
    virtual QString GetName() const = 0;

    // /**
    // * Returns the descriptor for the icon to show for this view.
    // */
    //virtual SmartPointer<ImageDescriptor> GetImageDescriptor() const = 0;

    /**
    * Equals this class with the given parameter.
    *
    * @param object the object for the equation
    * @return true, if the objects are equal :: false, if they differ in any way
    */
    bool operator==(const Object* object) const override = 0;

  }; // end struct IInputDeviceDescriptor
} //end namespace mitk

#endif /*MITKIINPUTDEVICEDESCRIPTOR_H_*/
