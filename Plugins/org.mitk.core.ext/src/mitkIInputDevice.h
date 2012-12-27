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

#ifndef MITKIINPUTDEVICE_H_
#define MITKIINPUTDEVICE_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <vector>

namespace mitk
{
  /**
  * An input device provides a method to register and unregister itself. Meaning <br>
  * for example adding listeners and instianciate classes necessary to use the input device.
  *
  * @noimplement This interface is not intended to be implemented by clients.
  * @ingroup org_mitk_core_ext
  */
  struct IInputDevice : public berry::Object
  {

    berryInterfaceMacro(IInputDevice, mitk)

    /**
    * Register the input device at one or more instances.
    */
    virtual bool RegisterInputDevice() = 0;

    /**
    * Unregister the input device at one or more instances.
    */
    virtual bool UnRegisterInputDevice() = 0;

    virtual ~IInputDevice() {}

  }; // end struct IInputDevice
} // end namespace mitk

Q_DECLARE_INTERFACE(mitk::IInputDevice, "org.mitk.IInputDevice")

#endif /*MITKIINPUTDEVICE_H_*/
