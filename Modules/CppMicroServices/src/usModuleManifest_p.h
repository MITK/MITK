/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEMANIFEST_P_H
#define USMODULEMANIFEST_P_H

#include <usAny.h>

namespace us {

/**
 * \brief Internal parser and container for a module's JSON manifest properties.
 *
 * This class is not part of the public API.
 *
 * \sa ModulePrivate ModuleInfo
 */
class ModuleManifest
{
public:

  /** \brief Construct an empty manifest. */
  ModuleManifest();

  /**
   * \brief Parse a JSON manifest from the given input stream.
   *
   * \param[in] is Input stream containing JSON manifest data.
   */
  void Parse(std::istream& is);

  /**
   * \brief Check whether a property with the given key exists.
   *
   * \param[in] key The property key to look up.
   * \return \c true if the key exists in the manifest.
   */
  bool Contains(const std::string& key) const;

  /**
   * \brief Retrieve the value associated with the given key.
   *
   * \param[in] key The property key.
   * \return The property value, or an empty Any if the key is absent.
   */
  Any GetValue(const std::string& key) const;

  /**
   * \brief Return all property keys in the manifest.
   *
   * \return A vector of key strings.
   */
  std::vector<std::string> GetKeys() const;

  /**
   * \brief Set or overwrite a property value.
   *
   * \param[in] key The property key.
   * \param[in] value The value to associate with the key.
   */
  void SetValue(const std::string& key, const Any& value);

private:

  typedef std::map<std::string, Any> AnyMap;

  AnyMap m_Properties; ///< \brief Stored manifest key-value pairs.
};

}

#endif // USMODULEMANIFEST_P_H
