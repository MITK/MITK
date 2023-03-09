/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIPreferencesStorage_h
#define mitkIPreferencesStorage_h

#include <mitkIPreferences.h>

#include <MitkCoreExports.h>

#include <filesystem>
#include <memory>

namespace mitk
{
  /**
   * \brief The backend for persistent preferences.
   *
   * This interface and its implementation is internally used by the IPreferencesService
   * to hold the preferences root node and to store and restore the preferences from disk.
   */
  class MITKCORE_EXPORT IPreferencesStorage
  {
  public:
    /**
     * \brief Constructor. Load preferences from the specified file.
     *
     * If the file does not yet exist, create an empty preferences storage.
     */
    explicit IPreferencesStorage(const std::filesystem::path& filename);

    /**
     * \brief Destructor. Write preferences to disk for the last time.
     */
    virtual ~IPreferencesStorage();

    /**
     * \brief Get the preferences root node.
     *
     * The preferences root node is owned by the preferences storage.
     */
    virtual IPreferences* GetRoot();

    /**
     * \sa GetRoot()
     */
    virtual const IPreferences* GetRoot() const;

    /**
     * \brief Get the filename of the preferences storage.
     */
    virtual std::filesystem::path GetFilename() const;

    /**
     * \brief Write the in-memory preferences to disk.
     *
     * Usually called by clients indirectly through IPreferences::Flush().
     */
    virtual void Flush() = 0;

  protected:
    std::filesystem::path m_Filename;
    std::unique_ptr<IPreferences> m_Root;
  };
}

#endif
