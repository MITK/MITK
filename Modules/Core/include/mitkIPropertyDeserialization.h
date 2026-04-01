/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIPropertyDeserialization_h
#define mitkIPropertyDeserialization_h

#include <mitkBaseProperty.h>
#include <mitkServiceInterface.h>
#include <MitkCoreExports.h>

#include <itkSmartPointer.h>

#include <string>
#include <type_traits>

namespace mitk
{
  /**
   * \ingroup MicroServices_Interfaces
   * \brief Interface of property deserialization service.
   *
   * This service allows you to register custom property types (derived from BaseProperty) for deserialization.
   * If a property type is not registered, it cannot be deserialized, e.g. when deserializing a PropertyList.
   */
  class MITKCORE_EXPORT IPropertyDeserialization
  {
  public:
    virtual ~IPropertyDeserialization();

    /** \brief Create a new instance of a registered property type by class name.
     *
     * \param className The class name of the property type to instantiate.
     * \return A new instance of the requested property type, or nullptr if not registered.
     */
    virtual BaseProperty::Pointer CreateInstance(const std::string& className) = 0;

    /**
     * \brief Register a custom property type for deserialization.
     *
     * The module activator of the module defining a property type is a good location to register
     * custom property types of that module. See the implementation of MitkCoreActivator for
     * examples.
     */
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseProperty, T>>>
    void RegisterProperty()
    {
      this->InternalRegisterProperty(T::New());
    }

  protected:
    /** \brief Internal method to register a property instance for deserialization.
     *
     * Called by the public RegisterProperty() template method.
     *
     * \param property A prototype instance of the property type to register.
     */
    virtual void InternalRegisterProperty(const BaseProperty* property) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPropertyDeserialization, "org.mitk.IPropertyDeserialization")

#endif
