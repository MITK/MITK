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
   * This class may be used to store properties similar to enumeration values.
   * Each enumeration value is identified by an id and a name. Note that both
   * name and id must be unique. Add enumeration values before you use the
   * Get/SetValue methods.
   *
   * To use this class, create a subclass that adds the possible enumeration
   * values in its constructor. You should override AddEnum() as protected so
   * that the user isn't able to add invalid enumeration values.
   *
   * As example see mitk::VtkRepresentationProperty or
   * mitk::VtkInterpolationProperty.
   *
   * @ingroup DataManagement
   */
  class MITKCORE_EXPORT EnumerationProperty : public BaseProperty
  {
  public:
    mitkClassMacro(EnumerationProperty, BaseProperty)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * Represents the unique id which is assigned to each enumeration name.
     */
    typedef unsigned int IdType;

    /**
     * Type used to store a mapping from enumeration id to enumeration name.
     */
    typedef std::map<IdType, std::string> EnumIdsContainerType;

    /**
     * Type used to store a mapping from enumeration name to enumeration id.
     */
    typedef std::map<std::string, IdType> EnumStringsContainerType;

    /**
     * Type used for iterators over all defined enumeration values.
     */
    typedef EnumIdsContainerType::const_iterator EnumConstIterator;

    /**
     * Adds an enumeration value into the enumeration. The name and id provided
     * must be unique. This is checked while adding the new enumeration value.
     * If it is not unique, false is returned. If addition was successful, true
     * is returned.
     * @param name The unique name of the enumeration value
     * @param id The unique id of the enumeration value
     * @returns True, if the name/id combination was successfully added to the
     * enumeration values. Otherwise false.
     */
    virtual bool AddEnum(const std::string &name, const IdType &id);

    /**
     * Sets the current value of the enumeration.
     * @param name The name of the enumeration value to set
     * @returns True if the value was successfully set. Otherwise false.
     */
    virtual bool SetValue(const std::string &name);

    /**
     * Sets the current value of the enumeration.
     * @param id The id of the enumeration value to set
     * @returns True, if the value was successfully set. Otherwise false.
     */
    virtual bool SetValue(const IdType &id);

    /**
     * Returns the id of the current enumeration value. If it was not set so far,
     * the return value is unspecified.
     */
    virtual IdType GetValueAsId() const;

    /**
     * Returns the name of the current enumeration value. If it was not set so far,
     * the return value is unspecified.
     */
    std::string GetValueAsString() const override;

    /**
     * Clears all enumerations including the current one.
     */
    virtual void Clear();

    /**
     * Determines the number of enumeration values.
     */
    virtual EnumIdsContainerType::size_type Size() const;

    /**
     * Provides access to the set of enumeration values. The name can be
     * accessed with iterator->second, the id via iterator->first.
     * @returns An iterator over all enumeration values.
     */
    virtual EnumConstIterator Begin() const;

    /**
     * Specifies the end of the range of enumeration values.
     * @returns An iterator pointing past the last enumeration values.
     */
    virtual EnumConstIterator End() const;

    /**
     * Returns the name for the given id.
     * @param id The id for which the name should be determined.
     *        If the id is invalid, the return value is unspecified.
     * @returns The name of the determined enumeration value.
     */
    virtual std::string GetEnumString(const IdType &id) const;

    /**
     * Returns the id for the given name.
     * @param name The enumeration name for which the id should be determined.
     *        If the name is invalid, the return value is unspecified.
     * @returns The id of the determined enumeration value.
     */
    virtual IdType GetEnumId(const std::string &name) const;

    /**
     * Determines if a given id is valid.
     * @param id The id to check
     * @returns True if the given id is valid. Otherwise false.
     */
    virtual bool IsValidEnumerationValue(const IdType &id) const;

    /**
     * Determines if a given name is valid.
     * @param name The name to check
     * @returns True if the given name is valid. Otherwise false.
     */
    virtual bool IsValidEnumerationValue(const std::string &name) const;

    const EnumIdsContainerType &GetEnumIds() const;
    const EnumStringsContainerType &GetEnumStrings() const;

    EnumIdsContainerType &GetEnumIds();
    EnumStringsContainerType &GetEnumStrings();

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

    mitkCloneMacro(Self)

  private:
    IdType m_CurrentValue;

    typedef std::map<std::string, EnumIdsContainerType> IdMapForClassNameContainerType;
    typedef std::map<std::string, EnumStringsContainerType> StringMapForClassNameContainerType;

    EnumIdsContainerType m_IdMap;
    EnumStringsContainerType m_NameMap;
  };
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#endif
