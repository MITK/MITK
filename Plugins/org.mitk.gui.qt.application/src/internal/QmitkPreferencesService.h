/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPreferencesService_h
#define QmitkPreferencesService_h

#include <mitkIPreferencesService.h>

/**
 * \brief Proxy for the core preferences service that additionally
 * provides the ability to open the preferences dialog.
 *
 * This service is registered with a higher service rank so that it
 * transparently replaces the default core preferences service when the
 * latter is requested via service lookup.
 *
 * The class forwards all calls to the underlying core preferences service,
 * but also implements OpenPreferencesDialog(), which requires Qt and
 * therefore cannot be provided by the core service itself.
 */
class QmitkPreferencesService : public mitk::IPreferencesService
{
public:
  QmitkPreferencesService();
  ~QmitkPreferencesService() override;

  void InitializeStorage(const fs::path& filename) override;
  void UninitializeStorage(bool removeFile) override;
  mitk::IPreferences* GetSystemPreferences() override;

  bool OpenPreferencesDialog(const std::string& page = {}) override;

private:
  // Raw, non-owning pointer; the core service is guaranteed to outlive this proxy.
  mitk::IPreferencesService* m_PreferencesService;
};

#endif
