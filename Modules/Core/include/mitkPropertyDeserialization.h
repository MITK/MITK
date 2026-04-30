/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyDeserialization_h
#define mitkPropertyDeserialization_h

#include <mitkIPropertyDeserialization.h>
#include <map>

namespace mitk
{
  /** \brief Implementation of the IPropertyDeserialization service interface.
   *
   * Maintains a registry of property type prototypes that can be used to
   * create new property instances during deserialization based on class name strings.
   *
   * \sa IPropertyDeserialization
   */
  class PropertyDeserialization : public IPropertyDeserialization
  {
  public:
    PropertyDeserialization();
    virtual ~PropertyDeserialization();

    PropertyDeserialization(const PropertyDeserialization&) = delete;
    PropertyDeserialization& operator=(const PropertyDeserialization&) = delete;

    /** \brief Create a new instance of a registered property type.
     *
     * Uses the stored prototype to create a new instance via CreateAnother().
     *
     * \param className The class name of the property type to instantiate.
     * \return A new property instance, or nullptr if the class name is not registered.
     */
    itk::SmartPointer<BaseProperty> CreateInstance(const std::string& className) override;

  protected:
    /** \brief Register a property prototype for deserialization.
     *
     * \param property The property prototype to register (keyed by its class name).
     */
    void InternalRegisterProperty(const BaseProperty* property) override;

  private:
    using MapType = std::map<std::string, BaseProperty::ConstPointer>;
    MapType m_Map;
  };
}

#endif
