/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPreferencesService_h
#define mitkPreferencesService_h

#include <mitkIPreferencesService.h>

#include <memory>

namespace mitk
{
  class IPreferencesStorage;

  /**
   * \brief Implementation of the IPreferencesService interface.
   *
   * Manages a single IPreferencesStorage instance that persists preferences to disk.
   * The storage is initialized lazily via InitializeStorage() and must only be
   * initialized once during the service lifetime. On destruction, any existing
   * storage is flushed to disk.
   *
   * Only used through the IPreferencesService interface.
   *
   * \sa IPreferencesService
   * \sa XMLPreferencesStorage
   * \sa IPreferences
   */
  class PreferencesService : public IPreferencesService
  {
  public:
    PreferencesService();

    /**
     * \brief Destructor that flushes pending preferences to disk.
     */
    ~PreferencesService() override;

    /**
     * \brief Initialize the backing storage from the given file.
     *
     * Must be called exactly once. A second call will throw an exception to
     * guarantee that preference pointers remain valid for the service lifetime.
     *
     * \param[in] filename Path to the preferences XML file.
     * \throw mitk::Exception if called more than once.
     */
    void InitializeStorage(const fs::path& filename) override;

    /**
     * \brief Uninitialize the storage and optionally remove the backing file.
     *
     * \param[in] removeFile If \c true, the preferences file is deleted from disk.
     */
    void UninitializeStorage(bool removeFile) override;

    /**
     * \brief Get the root preferences node.
     *
     * \return The root IPreferences node, or nullptr if storage is not initialized.
     */
    IPreferences* GetSystemPreferences() override;

  private:
    std::unique_ptr<IPreferencesStorage> m_Storage;
  };
}

#endif
