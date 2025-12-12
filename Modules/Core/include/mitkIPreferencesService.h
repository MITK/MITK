/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIPreferencesService_h
#define mitkIPreferencesService_h

#include <mitkServiceInterface.h>
#include <MitkCoreExports.h>

#include <mitkFileSystem.h>

namespace mitk
{
  class IPreferences;

  /**
   * \brief A service for persistent application preferences.
   *
   * \sa CoreServices::GetPreferencesService()
   * \sa IPreferences
   *
   * \ingroup MicroServices_Interfaces
   */
  class MITKCORE_EXPORT IPreferencesService
  {
  public:
    /**
     * \brief If initialized, ask the preferences backend to flush preferences, i.e. write them to disk.
     */
    virtual ~IPreferencesService();

    /**
     * \brief Initialize the preferences backend.
     *
     * Load preferences from the specified file. If the file does not yet exist,
     * create an empty preferences storage.
     *
     * This method must be called once at the start of an application before
     * accessing any preferences via GetSystemPreferences().
     *
     * \throw Exception The method is called more than once.
     *
     * \sa IPreferencesStorage
     */
    virtual void InitializeStorage(const fs::path& filename) = 0;

    /**
     * \brief For internal use only.
     *
     * This method is only used for testing purposes.
     * Do not use in production code.
     */
    virtual void UninitializeStorage(bool removeFile = true) = 0;

    /**
     * \brief Access preferences.
     *
     * The system preferences should be considered as resource and as such their
     * lifetime is coupled to this service. It is recommended to access the
     * preferences always through this method as needed instead of keeping
     * a pointer permanently.
     *
     * \note The term "system preferences" is kept for historical reasons to stay as
     * API-compatible as possible to the previous preferences service. A more
     * precise term would be "application preferences".
     *
     * \return The root node of the preferences tree.
     */
    virtual IPreferences* GetSystemPreferences() = 0;

    /**
     * \brief Opens the preferences dialog, if available.
     *
     * This method attempts to open the preferences dialog. The exact behavior
     * depends on the implementation - it may be non-blocking (e.g., using
     * `QDialog::open()`) or blocking until the dialog is closed (e.g., using
     * `QDialog::exec()`).
     *
     * \param page Optional. The ID of the preference page to display initially.
     *             This should correspond to a page ID defined in a plugin.xml file.
     *             If empty, the default preferences page will be shown.
     *
     * \return `true` if the preferences dialog was successfully opened;
               otherwise `false`.
     */
    virtual bool OpenPreferencesDialog(const std::string& page = {});
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IPreferencesService, "org.mitk.IPreferencesService")

#endif
