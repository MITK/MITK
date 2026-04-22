/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStringProperty_h
#define mitkStringProperty_h

#include <itkConfigure.h>

#include <mitkBaseProperty.h>
#include <MitkCoreExports.h>

#include <string>

namespace mitk
{
  /**
   * \brief Property for storing string values.
   *
   * This property holds a std::string value. It is one of the most commonly used
   * property types in MITK, for example to store names, labels, or other textual
   * metadata associated with data objects.
   *
   * \ingroup DataManagement
   *
   * \sa BaseProperty
   * \sa GenericProperty
   * \sa TemporoSpatialStringProperty
   */
  class MITKCORE_EXPORT StringProperty : public BaseProperty
  {
  protected:
    std::string m_Value;

    StringProperty(const char *string = nullptr);
    StringProperty(const std::string &s);

    StringProperty(const StringProperty &);

    mitkCloneMacro(StringProperty);

  public:
    mitkClassMacro(StringProperty, BaseProperty);

    /** \brief The type of the value stored by this property. */
    typedef std::string ValueType;

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkNewMacro1Param(StringProperty, const char*);
    mitkNewMacro1Param(StringProperty, const std::string&);

    /**
     * \brief Get the string value.
     * \return The current string value.
     */
    itkGetStringMacro(Value);

    /**
     * \brief Set the string value.
     * \param[in] _arg The new string value.
     */
    itkSetStringMacro(Value);

    /**
     * \brief Return the property value as a string.
     * \return The stored string value.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Serialize the string value to JSON.
     *
     * \param[out] j The JSON object to write the value into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the string value from JSON.
     *
     * \param[in] j The JSON value containing the string.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    /** \brief Constant used as a convention for properties representing file paths. */
    static const char *PATH;

    using BaseProperty::operator=;

  private:
    // purposely not implemented
    StringProperty &operator=(const StringProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

} // namespace mitk

#endif
