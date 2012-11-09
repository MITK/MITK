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

#ifndef MITKSPACENAVIGATORREGISTRY_H_
#define MITKSPACENAVIGATORREGISTRY_H_

#include <Poco/HashMap.h>
#include <string>
#include <vector>

#include <mitkIInputDeviceDescriptor.h>
#include <mitkIInputDeviceRegistry.h>

namespace mitk
{
  /**
  * Documentation in the interface.
  *
  * @see mitk::IInputDeviceRegistry
  * @ingroup org_mitk_core_ext
  */
  class InputDeviceRegistry : public IInputDeviceRegistry
  {
  public:

    bool IsA(const std::type_info& type) const;
    const std::type_info& GetType() const;

    // easier maintenance
    typedef IInputDeviceDescriptor::Pointer InputDeviceDescriptorPtr;

    InputDeviceRegistry();
    ~InputDeviceRegistry();

    /**
    * @see mitk::IInputDeviceRegistry::Find(const std::string& id)
    */
    InputDeviceDescriptorPtr Find(const std::string& id) const;

    /**
    * @see mitk::IInputDeviceRegistry´::GetInputDevices()
    */
    std::vector<InputDeviceDescriptorPtr> GetInputDevices() const;

  protected:

  private:
    Poco::HashMap<std::string, InputDeviceDescriptorPtr> m_ListRegisteredDevices;

  }; // end class InputDeviceRegistry
} // end namespace mitk

#endif /*MITKSPACENAVIGATORREGISTRY_H_*/
