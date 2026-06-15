/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseProperty_h
#define mitkBaseProperty_h

#include <MitkCoreExports.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <string>
#include <nlohmann/json_fwd.hpp>

namespace mitk
{
  /**
   * \brief Abstract base class for properties.
   *
   * Base class for all properties in MITK. Properties represent arbitrary additional
   * information that can be added to a PropertyList and associated with data objects.
   * To define a new type of property, create a subclass of BaseProperty.
   *
   * Concrete subclasses should define Set-/Get-methods to access the property value,
   * which should be stored by value (not by reference). Subclasses must implement the
   * pure virtual methods IsEqual() and Assign() for comparison and assignment support.
   *
   * \sa PropertyList
   * \sa GenericProperty
   * \sa StringProperty
   */
  class MITKCORE_EXPORT BaseProperty : public itk::Object
  {
  public:
    mitkClassMacroItkParent(BaseProperty, itk::Object);
    itkCloneMacro(Self);

    /**
     * \brief Compare two properties for equality.
     *
     * Checks whether this property is equal to the given property. Two properties are
     * considered equal if they have the same type (checked via typeid) and their
     * subclass-specific IsEqual() method returns true.
     *
     * \param[in] property The property to compare against.
     * \return \c true if both properties are of the same type and have equal values,
     *         \c false otherwise.
     *
     * \sa IsEqual
     */
    bool operator==(const BaseProperty &property) const;

    /**
     * \brief Assign a property value to this instance.
     *
     * Performs a polymorphic assignment from the given property. The assignment succeeds
     * only if the source and target properties are of the same type. The subclass-specific
     * Assign() method is called to perform the actual value copy.
     *
     * Subclasses must implement Assign(const BaseProperty&) and call the superclass
     * Assign method for proper handling. The copy assignment operator of the subclass
     * should be disabled and the base class operator should be made visible using
     * "using" statements.
     *
     * \param[in] property The property whose value should be assigned to this instance.
     * \return A reference to this property.
     *
     * \sa AssignProperty
     * \sa Assign
     */
    BaseProperty &operator=(const BaseProperty &property);

    /**
     * \brief Assign a property value to this instance and report success.
     *
     * This method is identical to the assignment operator, except for the return type.
     * It allows callers to directly check whether the assignment was successful.
     * The assignment fails if the source and target properties have different types.
     * On success, the property is marked as modified.
     *
     * \param[in] property The property whose value should be assigned to this instance.
     * \return \c true if the assignment was successful (same type), \c false otherwise.
     *
     * \sa operator=
     */
    bool AssignProperty(const BaseProperty &property);

    /**
     * \brief Return the property value as a human-readable string.
     *
     * The default implementation returns VALUE_CANNOT_BE_CONVERTED_TO_STRING.
     * Subclasses should override this method to provide a meaningful string
     * representation of their value.
     *
     * \return A string representation of the property value.
     */
    virtual std::string GetValueAsString() const;

    /**
     * \brief Serialize property value(s) to JSON.
     *
     * Subclasses must implement this method to serialize their value into the
     * provided JSON object. Rely on exceptions for error handling when
     * implementing serialization.
     *
     * \param[out] j The JSON object to write the serialized value into.
     * \return \c false if the property is not serializable by design, \c true otherwise.
     */
    virtual bool ToJSON(nlohmann::json& j) const = 0;

    /**
     * \brief Deserialize property value(s) from JSON.
     *
     * Subclasses must implement this method to restore their value from the
     * provided JSON object. Rely on exceptions for error handling when
     * implementing deserialization.
     *
     * \param[in] j The JSON object containing the serialized value.
     * \return \c false if the property is not deserializable by design, \c true otherwise.
     */
    virtual bool FromJSON(const nlohmann::json& j) = 0;

    /**
     * \brief Default return value for properties that cannot be converted to a string.
     *
     * This constant (value "n/a") is returned by GetValueAsString() in the base
     * implementation to indicate that no string conversion is available.
     */
    static const std::string VALUE_CANNOT_BE_CONVERTED_TO_STRING;

  protected:
    BaseProperty();
    BaseProperty(const BaseProperty &other);

    ~BaseProperty() override;

  private:
    /*!
      Override this method in subclasses to implement a meaningful comparison. The property
      argument is guaranteed to be castable to the type of the implementing subclass.
    */
    virtual bool IsEqual(const BaseProperty &property) const = 0;

    /*!
      Override this method in subclasses to implement a meaningful assignment. The property
      argument is guaranteed to be castable to the type of the implementing subclass.

      @warning This is not yet exception aware/safe and if this method returns false,
               this property's state might be undefined.

      @return True if the argument could be assigned to this property.
     */
    virtual bool Assign(const BaseProperty &) = 0;
  };

  /**
   * @brief Serialize a property to self contained JSON.
   *
   * This is a helper methods that converts the property in a self contained json (so
   * nothing more is needed to serialize or deserialize w/o any loss.
   * Simple property types (StringProperty, IntProperty, FloatProperty, BoolProperty)
   * are serialized directly as their JSON primitive values.
   * Complex types are serialized as objects with "type" and "value" fields.
   *
   * JSON format:
   * - Simple types (string, int, float, bool) serialize directly as their JSON value
   * - Complex types use: {"type": "ClassName", "value": {...}}
   *
   * @param property The property to serialize. Must not be nullptr.
   * @return JSON representation of the property.
   * @throws mitk::Exception if property is nullptr or does not support JSON serialization.
   */
  MITKCORE_EXPORT nlohmann::json ConvertPropertyToSelfContainedJson(const BaseProperty* property);

  /**
   * @brief Deserialize a property from self contained JSON representation.
   *
   * This is a helper methods that converts the property from a self contained json (so
   * nothing more is needed to serialize or deserialize w/o any loss.
   * Handles both simple JSON values (string, int, float, bool) and complex
   * property objects with "type" and "value" fields.
   *
   * For simple values, the type is inferred from the JSON type:
   * - JSON string -> StringProperty
   * - JSON integer -> IntProperty
   * - JSON float -> FloatProperty
   * - JSON boolean -> BoolProperty
   *
   * For complex types, uses IPropertyDeserialization service to create instances.
   *
   *
   * JSON format:
   * - Simple types (string, int, float, bool) serialize directly as their JSON value
   * - Complex types use: {"type": "ClassName", "value": {...}}
   *
   * @param json The JSON value to deserialize.
   * @return Created property.
   * @throws mitk::Exception if JSON type is unsupported or deserialization fails.
   */
  MITKCORE_EXPORT BaseProperty::Pointer ConvertPropertyFromSelfContainedJson(const nlohmann::json& json);

} // namespace mitk

#endif
