/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkXMLPreferencesStorage_h
#define mitkXMLPreferencesStorage_h

#include <mitkIPreferencesStorage.h>

namespace mitk
{
  /**
   * \brief XML file-backed implementation of IPreferencesStorage.
   *
   * Persists the preferences tree to an XML file using TinyXML2. On construction,
   * loads existing preferences from the file if it exists, otherwise creates
   * an empty root node and ensures the parent directory exists. On Flush(),
   * serializes the entire preferences tree back to the file.
   *
   * \sa IPreferencesStorage
   * \sa Preferences
   * \sa PreferencesService
   */
  class XMLPreferencesStorage : public IPreferencesStorage
  {
  public:
    /**
     * \brief Construct storage backed by the given XML file.
     *
     * If the file exists, its contents are deserialized into the preferences tree.
     * Otherwise, parent directories are created and an empty root node is initialized.
     *
     * \param[in] filename Path to the XML preferences file.
     */
    explicit XMLPreferencesStorage(const fs::path& filename);

    ~XMLPreferencesStorage() override;

    /**
     * \brief Serialize the entire preferences tree to the backing XML file.
     *
     * Overwrites the file with the current state of all preferences nodes.
     */
    void Flush() override;
  };
}

#endif
