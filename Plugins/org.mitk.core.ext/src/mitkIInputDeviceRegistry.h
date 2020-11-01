/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIINPUTDEVICEREGISTRY_H_
#define MITKIINPUTDEVICEREGISTRY_H_

#include <QtPlugin>

#include "mitkIInputDeviceDescriptor.h"

namespace mitk
{
  /**
   *
   * The input device registry maintains a list of input devices explicitly registered
   * against the view extension point.
   * <p>
   * The description of a given input device is kept in a <code>IInputDeviceDescriptor</code>.
   * </p>
   * <p>
   * This interface is not intended to be implemented by clients.
   * </p>
   *
   * @see mitk::IInputDeviceDescriptor
   * @note This interface is not intended to be implemented by clients.
   * @ingroup org_mitk_core_ext
   */
  struct IInputDeviceRegistry
  {

    /**
     * Return an input device descriptor with the given extension id. If no input device exists,
     * with the id return <code>null</code>.
     *
     * @param id
     *      the id to search for
     * @return the descriptor or <code>null</code>
     */
    virtual IInputDeviceDescriptor::Pointer Find(const QString& id) const = 0;

    /**
     * Return a list of input devices defined in the registry.
     *
     * @return the input devices.
     */
    virtual QList<IInputDeviceDescriptor::Pointer> GetInputDevices() const = 0;

    virtual ~IInputDeviceRegistry() {}

  }; // end struct IInputDeviceRegistry
} // end namespace mitk

Q_DECLARE_INTERFACE(mitk::IInputDeviceRegistry, "org.mitk.service.IInputDeviceRegistry")

#endif /*MITKIINPUTDEVICEREGISTRY_H_*/
