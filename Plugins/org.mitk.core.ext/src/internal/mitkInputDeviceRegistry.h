/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKSPACENAVIGATORREGISTRY_H_
#define MITKSPACENAVIGATORREGISTRY_H_

#include <mitkIInputDeviceDescriptor.h>
#include <mitkIInputDeviceRegistry.h>

#include <QHash>

namespace mitk
{
  /**
  * Documentation in the interface.
  *
  * @see mitk::IInputDeviceRegistry
  * @ingroup org_mitk_core_ext
  */
  class InputDeviceRegistry : public QObject, public IInputDeviceRegistry
  {
    Q_OBJECT
    Q_INTERFACES(mitk::IInputDeviceRegistry)

  public:

    // easier maintenance
    typedef IInputDeviceDescriptor::Pointer InputDeviceDescriptorPtr;

    InputDeviceRegistry();
    ~InputDeviceRegistry() override;

    /**
    * @see mitk::IInputDeviceRegistry::Find(const std::string& id)
    */
    InputDeviceDescriptorPtr Find(const QString& id) const override;

    /**
    * @see mitk::IInputDeviceRegistry´::GetInputDevices()
    */
    QList<InputDeviceDescriptorPtr> GetInputDevices() const override;

  protected:

  private:
    QHash<QString, InputDeviceDescriptorPtr> m_ListRegisteredDevices;

  }; // end class InputDeviceRegistry
} // end namespace mitk

#endif /*MITKSPACENAVIGATORREGISTRY_H_*/
