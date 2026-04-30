/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkConfigurationHolder_h
#define mitkConfigurationHolder_h

#include <MitkCLCoreExports.h>

//#include <mitkBaseData.h>

// STD Includes
#include <string>
#include <map>
#include <vector>

namespace mitk
{
  /**
   * \brief A variant-like container for storing typed configuration values.
   *
   * ConfigurationHolder stores a single value of one of several supported types
   * (bool, unsigned int, int, double, string) or a named group of child
   * ConfigurationHolder objects. The stored type is tracked internally, and
   * accessor methods attempt automatic type conversion when possible.
   *
   * When a value has not been set (DT_UNINIZIALIZED) or when a group value
   * cannot be converted, the accessor methods throw a mitk::Exception.
   *
   * \sa ConfigFileReader
   * \sa AbstractGlobalImageFeature
   */
  class MITKCLCORE_EXPORT ConfigurationHolder // : public BaseData
  {
  public:
    /**
     * \brief Enumerates the data types that a ConfigurationHolder can store.
     */
    enum ValueType
    {
      DT_UNINIZIALIZED, ///< No value has been set yet.
      DT_BOOL,          ///< Boolean value.
      DT_UINT,          ///< Unsigned integer value.
      DT_INT,           ///< Signed integer value.
      DT_DOUBLE,        ///< Double-precision floating-point value.
      DT_STRING,        ///< String value.
      DT_GROUP          ///< Group of named ConfigurationHolder children.
    };

    /** \brief Construct an uninitialized ConfigurationHolder. */
    ConfigurationHolder();

    /**
     * \brief Store a boolean value.
     * \param[in] value The boolean value to store.
     */
    void SetBool(bool value);

    /**
     * \brief Store an unsigned integer value.
     * \param[in] value The unsigned integer value to store.
     */
    void SetUnsignedInt(unsigned int value);

    /**
     * \brief Store a signed integer value.
     * \param[in] value The signed integer value to store.
     */
    void SetInt(int value);

    /**
     * \brief Store a double-precision floating-point value.
     * \param[in] value The double value to store.
     */
    void SetDouble(double value);

    /**
     * \brief Store a string value.
     * \param[in] value The string value to store.
     */
    void SetString(std::string value);

    /**
     * \brief Clear all children from the group and set the type to DT_GROUP.
     */
    void ClearGroup();

    /**
     * \brief Add a named child to the group.
     *
     * Sets the value type to DT_GROUP.
     *
     * \param[in] id The identifier for the child entry.
     * \param[in] value The ConfigurationHolder to add as a child.
     */
    void AddToGroup(std::string id, const ConfigurationHolder &value);

    /**
     * \brief Convert the stored value to bool.
     * \return The value converted to bool.
     * \throw mitk::Exception If the value is uninitialized or is group data.
     */
    bool AsBool();

    /**
     * \brief Convert the stored value to unsigned int.
     * \return The value converted to unsigned int.
     * \throw mitk::Exception If the value is uninitialized or is group data.
     */
    unsigned int AsUnsignedInt();

    /**
     * \brief Convert the stored value to int.
     * \return The value converted to int.
     * \throw mitk::Exception If the value is uninitialized or is group data.
     */
    int AsInt();

    /**
     * \brief Convert the stored value to double.
     * \return The value converted to double.
     * \throw mitk::Exception If the value is uninitialized or is group data.
     */
    double AsDouble();

    /**
     * \brief Convert the stored value to string.
     * \return The value converted to string.
     * \throw mitk::Exception If the value is uninitialized or is group data.
     */
    std::string AsString();

    /**
     * \brief Convert the stored value to bool, returning a default on failure.
     * \param[in] value Default value returned if conversion fails.
     * \return The converted value, or the default if a mitk::Exception is thrown.
     */
    bool AsBool(bool value);

    /**
     * \brief Convert the stored value to unsigned int, returning a default on failure.
     * \param[in] value Default value returned if conversion fails.
     * \return The converted value, or the default if a mitk::Exception is thrown.
     */
    unsigned int AsUnsignedInt(unsigned int value);

    /**
     * \brief Convert the stored value to int, returning a default on failure.
     * \param[in] value Default value returned if conversion fails.
     * \return The converted value, or the default if a mitk::Exception is thrown.
     */
    int AsInt(int value);

    /**
     * \brief Convert the stored value to double, returning a default on failure.
     * \param[in] value Default value returned if conversion fails.
     * \return The converted value, or the default if a mitk::Exception is thrown.
     */
    double AsDouble(double value);

    /**
     * \brief Convert the stored value to string, returning a default on failure.
     * \param[in] value Default value returned if conversion fails.
     * \return The converted value, or the default if a mitk::Exception is thrown.
     */
    std::string AsString(std::string value);

    /**
     * \brief Convert group children to a vector of strings.
     *
     * Each child's value is converted to string via AsString().
     *
     * \return A vector of string representations of all group children.
     * \throw mitk::Exception If the value type is not DT_GROUP.
     */
    std::vector<std::string> AsStringVector();

    /**
     * \brief Access a child ConfigurationHolder by identifier.
     *
     * If no child with the given id exists, a default-constructed entry is created.
     *
     * \param[in] id The identifier of the child to access.
     * \return Reference to the child ConfigurationHolder.
     */
    ConfigurationHolder& At(std::string id);

  private:
    bool m_BoolValue;
    unsigned int m_UIntValue;
    int m_IntValue;
    double m_DoubleValue;
    std::string m_StringValue;

    std::map<std::string, ConfigurationHolder> m_GroupValue;

    ValueType m_ValueType;
  };
}

#endif
