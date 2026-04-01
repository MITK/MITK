/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEnumerationProperty_h
#define mitkEnumerationProperty_h

#include <mitkBaseProperty.h>

#include <map>
#include <string>

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4522) // "multiple assignment operators specified"
#endif

namespace mitk
{
  /**
   * \brief Property for storing enumeration-like values identified by id and name.
   *
   * This class may be used to store properties similar to C++ enumeration values.
   * Each enumeration value is identified by a unique id and a unique name. You must
   * add enumeration values before using the Get/SetValue methods.
   *
   * To use this class, create a subclass that adds the possible enumeration
   * values in its constructor. You should override AddEnum() as protected so
   * that users cannot add invalid enumeration values.
   *
   * \ingroup DataManagement
   *
   * \sa VtkRepresentationProperty
   * \sa VtkInterpolationProperty
   * \sa BaseProperty
   */
  class MITKCORE_EXPORT EnumerationProperty : public BaseProperty
  {
  public:
    mitkClassMacro(EnumerationProperty, BaseProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Unsigned integer type used as unique enumeration value identifier. */
    typedef unsigned int IdType;

    /** \brief Map type storing id-to-name mappings. */
    typedef std::map<IdType, std::string> EnumIdsContainerType;

    /** \brief Map type storing name-to-id mappings. */
    typedef std::map<std::string, IdType> EnumStringsContainerType;

    /** \brief Const iterator type for iterating over enumeration values. */
    typedef EnumIdsContainerType::const_iterator EnumConstIterator;

    /**
     * \brief Add an enumeration value.
     *
     * The name and id provided must both be unique. If either already exists,
     * the value is not added and \c false is returned.
     *
     * \param[in] name The unique name of the enumeration value.
     * \param[in] id The unique id of the enumeration value.
     * \return \c true if the name/id combination was successfully added,
     *         \c false if either name or id already exists.
     */
    virtual bool AddEnum(const std::string &name, const IdType &id);

    /**
     * \brief Set the current value of the enumeration by name.
     *
     * \param[in] name The name of the enumeration value to set.
     * \return \c true if the value was successfully set (name is valid),
     *         \c false otherwise.
     */
    virtual bool SetValue(const std::string &name);

    /**
     * \brief Set the current value of the enumeration by id.
     *
     * \param[in] id The id of the enumeration value to set.
     * \return \c true if the value was successfully set (id is valid),
     *         \c false otherwise.
     */
    virtual bool SetValue(const IdType &id);

    /**
     * \brief Get the id of the currently selected enumeration value.
     *
     * \return The id of the current value. If no value has been set, the
     *         return value is unspecified.
     */
    virtual IdType GetValueAsId() const;

    /**
     * \brief Get the name of the currently selected enumeration value.
     *
     * \return The name string of the current value. If no value has been set,
     *         the return value is unspecified.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Remove all enumeration values and reset the current selection.
     */
    virtual void Clear();

    /**
     * \brief Get the number of defined enumeration values.
     *
     * \return The count of registered enumeration values.
     */
    virtual EnumIdsContainerType::size_type Size() const;

    /**
     * \brief Get an iterator to the first enumeration value.
     *
     * Iterator dereferences to a pair where \c first is the id and \c second is the name.
     *
     * \return A const iterator to the beginning of the enumeration values.
     */
    virtual EnumConstIterator Begin() const;

    /**
     * \brief Get an iterator past the last enumeration value.
     *
     * \return A const iterator pointing past the end of the enumeration values.
     */
    virtual EnumConstIterator End() const;

    /**
     * \brief Get the name string for a given enumeration id.
     *
     * \param[in] id The id for which to retrieve the name.
     * \return The name of the enumeration value. If the id is invalid, the
     *         return value is the string "invalid enum id or enums empty".
     */
    virtual std::string GetEnumString(const IdType &id) const;

    /**
     * \brief Get the id for a given enumeration name.
     *
     * \param[in] name The enumeration name for which to retrieve the id.
     * \return The id of the enumeration value. If the name is invalid,
     *         the return value is 0.
     */
    virtual IdType GetEnumId(const std::string &name) const;

    /**
     * \brief Check whether a given id corresponds to a valid enumeration value.
     *
     * \param[in] id The id to check.
     * \return \c true if the id is valid, \c false otherwise.
     */
    virtual bool IsValidEnumerationValue(const IdType &id) const;

    /**
     * \brief Check whether a given name corresponds to a valid enumeration value.
     *
     * \param[in] name The name to check.
     * \return \c true if the name is valid, \c false otherwise.
     */
    virtual bool IsValidEnumerationValue(const std::string &name) const;

    /**
     * \brief Get a const reference to the id-to-name map.
     * \return The map of enumeration ids to names.
     */
    const EnumIdsContainerType &GetEnumIds() const;

    /**
     * \brief Get a const reference to the name-to-id map.
     * \return The map of enumeration names to ids.
     */
    const EnumStringsContainerType &GetEnumStrings() const;

    /**
     * \brief Get a mutable reference to the id-to-name map.
     * \return The map of enumeration ids to names.
     */
    EnumIdsContainerType &GetEnumIds();

    /**
     * \brief Get a mutable reference to the name-to-id map.
     * \return The map of enumeration names to ids.
     */
    EnumStringsContainerType &GetEnumStrings();

    /**
     * \brief Serialize the current enumeration value to JSON.
     *
     * Writes the current enumeration value name as a JSON string.
     *
     * \note Classes deriving from EnumerationProperty are covered by this
     * implementation and do not need to override this method again.
     *
     * \param[out] j The JSON value to write into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the enumeration value from JSON.
     *
     * Reads a string from JSON and sets the enumeration value by name.
     *
     * \note Classes deriving from EnumerationProperty are covered by this
     * implementation and do not need to override this method again.
     *
     * \param[in] j The JSON string value to read from.
     * \return Always \c true.
     * \throw mitk::Exception if the JSON string is not a valid enumeration value.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;
    EnumerationProperty & operator=(const EnumerationProperty &) = delete;

  protected:
    /**
     * Default constructor. The current value of the enumeration is undefined.
     */
    EnumerationProperty();

    EnumerationProperty(const EnumerationProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;

    mitkCloneMacro(Self);

  private:
    IdType m_CurrentValue;
    EnumIdsContainerType m_IdMap;
    EnumStringsContainerType m_NameMap;
  };
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#endif
