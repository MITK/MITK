/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGenericProperty_h
#define mitkGenericProperty_h

#include <sstream>
#include <cstdlib>
#include <string>

#include <mitkBaseProperty.h>
#include <mitkNumericTypes.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Template class for generating properties for simple value types (int, float, bool, etc.).
   *
   * This class template can be instantiated for all classes/internal types that fulfill
   * these requirements:
   *   - an \c operator<< so that the property value can be put into a std::stringstream
   *   - an \c operator== so that two properties can be checked for equality
   *
   * \note You must use the macros mitkDeclareGenericProperty and mitkDefineGenericProperty to
   * provide specializations for concrete types (e.g. BoolProperty). See mitkProperties.h for
   * examples. If you don't use these macros, GetNameOfClass() will return "GenericProperty",
   * which will break serialization.
   *
   * \tparam T The value type stored by this property.
   *
   * \ingroup DataManagement
   *
   * \sa BaseProperty
   * \sa mitkDeclareGenericProperty
   * \sa mitkDefineGenericProperty
   */
  template <typename T>
  class MITK_EXPORT GenericProperty : public BaseProperty
  {
  public:
    mitkClassMacro(GenericProperty, BaseProperty);
    mitkNewMacro1Param(GenericProperty<T>, T);
    itkCloneMacro(Self);

    /** \brief The type of the value stored by this property. */
    typedef T ValueType;

    /** \brief Set the property value.
     *  \param[in] _arg The new value. */
    itkSetMacro(Value, T);

    /** \brief Get the property value.
     *  \return The current value. */
    itkGetConstMacro(Value, T);

    /**
     * \brief Return the property value as a string.
     *
     * Uses \c operator<< to stream the value into a stringstream.
     *
     * \return A string representation of the property value.
     */
    std::string GetValueAsString() const override
    {
      std::stringstream myStr;
      myStr << GetValue();
      return myStr.str();
    }

    /**
     * \brief Serialize property value to JSON.
     *
     * The base GenericProperty template does not support JSON serialization.
     * Concrete specializations (via mitkDefineGenericProperty) override this
     * to provide actual serialization.
     *
     * \return Always \c false in the base template.
     */
    bool ToJSON(nlohmann::json&) const override
    {
      return false;
    }

    /**
     * \brief Deserialize property value from JSON.
     *
     * The base GenericProperty template does not support JSON deserialization.
     * Concrete specializations (via mitkDefineGenericProperty) override this
     * to provide actual deserialization.
     *
     * \return Always \c false in the base template.
     */
    bool FromJSON(const nlohmann::json&) override
    {
      return false;
    }

    using BaseProperty::operator=;

  protected:
    GenericProperty() {}
    GenericProperty(T x) : m_Value(x) {}
    GenericProperty(const GenericProperty &other) : BaseProperty(other), m_Value(other.m_Value) {}
    T m_Value;

    mitkCloneMacro(Self);

  private:
    // purposely not implemented
    GenericProperty &operator=(const GenericProperty &);

    bool IsEqual(const BaseProperty &other) const override
    {
      return (this->m_Value == static_cast<const Self &>(other).m_Value);
    }

    bool Assign(const BaseProperty &other) override
    {
      this->m_Value = static_cast<const Self &>(other).m_Value;
      return true;
    }
  };


} // namespace mitk

/**
 * Generates a specialized subclass of mitk::GenericProperty.
 * This way, GetNameOfClass() returns the value provided by PropertyName.
 * Please see mitkProperties.h for examples.
 * @param PropertyName the name of the subclass of GenericProperty
 * @param Type the value type of the GenericProperty
 * @param Export the export macro for DLL usage
 */
#define mitkDeclareGenericProperty(PropertyName, Type, Export)                                                         \
                                                                                                                       \
  class Export PropertyName : public GenericProperty<Type>                                                             \
                                                                                                                       \
  {                                                                                                                    \
  public:                                                                                                              \
    mitkClassMacro(PropertyName, GenericProperty<Type>);                                                               \
    itkFactorylessNewMacro(Self);                                                                                      \
    itkCloneMacro(Self);                                                                                               \
    mitkNewMacro1Param(PropertyName, Type);                                                                            \
                                                                                                                       \
    bool ToJSON(nlohmann::json& j) const override;                                                                     \
    bool FromJSON(const nlohmann::json& j) override;                                                                   \
                                                                                                                       \
    using BaseProperty::operator=;                                                                                     \
                                                                                                                       \
  protected:                                                                                                           \
    PropertyName();                                                                                                    \
    PropertyName(const PropertyName &);                                                                                \
    PropertyName(Type x);                                                                                              \
                                                                                                                       \
    mitkCloneMacro(Self);                                                                                              \
  };

#define mitkDefineGenericProperty(PropertyName, Type, DefaultValue)                                                    \
  mitk::PropertyName::PropertyName() : Superclass(DefaultValue) {}                                                     \
  mitk::PropertyName::PropertyName(const PropertyName &other) : GenericProperty<Type>(other) {}                        \
  mitk::PropertyName::PropertyName(Type x) : Superclass(x) {}                                                          \
  bool mitk::PropertyName::ToJSON(nlohmann::json& j) const                                                             \
  {                                                                                                                    \
    j = this->GetValue();                                                                                              \
    return true;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  bool mitk::PropertyName::FromJSON(const nlohmann::json& j)                                                           \
  {                                                                                                                    \
    this->SetValue(j.get<Type>());                                                                                     \
    return true;                                                                                                       \
  }

#endif
