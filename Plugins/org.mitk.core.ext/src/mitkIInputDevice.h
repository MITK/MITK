/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  * @note This interface is not intended to be implemented by clients.
  * @ingroup org_mitk_core_ext
  */
  struct IInputDevice : public berry::Object
  {

    berryObjectMacro(mitk::IInputDevice);

    /**
    * Register the input device at one or more instances.
    */
    virtual bool RegisterInputDevice() = 0;

    /**
    * Unregister the input device at one or more instances.
    */
    virtual bool UnRegisterInputDevice() = 0;

    ~IInputDevice() override {}

  }; // end struct IInputDevice
} // end namespace mitk

Q_DECLARE_INTERFACE(mitk::IInputDevice, "org.mitk.IInputDevice")

#endif /*MITKIINPUTDEVICE_H_*/
