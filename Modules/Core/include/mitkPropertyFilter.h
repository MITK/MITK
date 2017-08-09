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

#ifndef mitkPropertyFilter_h
#define mitkPropertyFilter_h

#include <MitkCoreExports.h>
#include <map>
#include <mitkBaseProperty.h>
#include <string>

namespace mitk
{
  /** \brief Consists of blacklist and whitelist entries.
    *
    * Blacklist filtering is less restrictive than whitelist filtering since you can filter out individual property
   * names.
    * If whitelist entries were added, all other property names are filtered out.
    * In addition, whitelist entries are reduced by blacklist entries.
    */
  class MITKCORE_EXPORT PropertyFilter
  {
  public:
    /** \brief Specifies the type of a filter entry.
      */
    enum List
    {
      Blacklist, /**< Blacklisted filter entries are filtered out. */
      Whitelist  /**< Whitelisted filter entries are the only entries that remain after filtering. */
    };

    PropertyFilter();
    ~PropertyFilter();

    PropertyFilter(const PropertyFilter &other);
    PropertyFilter &operator=(PropertyFilter other);

    /** \brief Add a filter entry for a specific property.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] list Type of the filter entry.
      */
    void AddEntry(const std::string &propertyName, List list);

    /** \brief Apply the filter to a property list.
      *
      * \param[in] propertyMap Property list to which the filter is applied.
      * \return Filtered property list.
      */
    std::map<std::string, BaseProperty::Pointer> Apply(
      const std::map<std::string, BaseProperty::Pointer> &propertyMap) const;

    /** \brief Check if filter has specific entry.
      *
      * \param[in] propertyName Name of the property.
      * \param[in] list Type of the filter entry.
      * \return True if property filter has specified entry, false otherwise.
      */
    bool HasEntry(const std::string &propertyName, List list) const;

    /** \brief Check if filter is empty.
      *
      * \return True if filter is empty, false otherwise.
      */
    bool IsEmpty() const;

    /** \brief Remove all entries from property filter.
      *
      * \param[in] list Filter list from which all entries are removed.
      */
    void RemoveAllEntries(List list);

    /** \brief Remove specific entry from property filter.
      *
      * \param[in] propertyName Name of property.
      * \param[in] list Filter list from which the entry is removed.
      */
    void RemoveEntry(const std::string &propertyName, List list);

  private:
    class Impl;
    Impl *m_Impl;
  };
}

#endif
