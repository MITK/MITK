/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEPROPERTIESIMPL_P_H
#define USSERVICEPROPERTIESIMPL_P_H

#include <usServiceProperties.h>

namespace us {

/**
 * \brief Private implementation of service properties storage.
 *
 * Stores service property keys and values in parallel vectors, supporting
 * both case-insensitive and case-sensitive key lookup.
 *
 * \sa ServiceProperties
 */
class ServicePropertiesImpl
{

public:

  /** \brief Construct from a ServiceProperties map.
   *  \param[in] props The service properties to store.
   */
  explicit ServicePropertiesImpl(const ServiceProperties& props);

  /** \brief Get a property value by case-insensitive key lookup.
   *  \param[in] key The property key to look up.
   *  \return The property value, or an empty Any if not found.
   */
  const Any& Value(const std::string& key) const;

  /** \brief Get a property value by index.
   *  \param[in] index The index of the property.
   *  \return The property value at the given index.
   */
  const Any& Value(int index) const;

  /** \brief Find the index of a property key (case-insensitive).
   *  \param[in] key The property key to search for.
   *  \return The index of the key, or -1 if not found.
   */
  int Find(const std::string& key) const;

  /** \brief Find the index of a property key (case-sensitive).
   *  \param[in] key The property key to search for.
   *  \return The index of the key, or -1 if not found.
   */
  int FindCaseSensitive(const std::string& key) const;

  /** \brief Get all property keys.
   *  \return A vector of property key strings.
   */
  const std::vector<std::string>& Keys() const;

private:

  /** \brief The property keys. */
  std::vector<std::string> keys;

  /** \brief The property values, parallel to keys. */
  std::vector<Any> values;

  /** \brief Sentinel empty Any returned for missing keys. */
  static Any emptyAny;

};

}

#endif // USSERVICEPROPERTIESIMPL_P_H
