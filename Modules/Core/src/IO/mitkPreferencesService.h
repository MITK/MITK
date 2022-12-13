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
   * Only used through the IPreferencesService interface.
   *
   * \sa IPreferencesService
   */
  class PreferencesService : public IPreferencesService
  {
  public:
    PreferencesService();
    ~PreferencesService() override;

    void InitializeStorage(const std::filesystem::path& filename) override;
    void UninitializeStorage(bool removeFile) override;
    IPreferences* GetSystemPreferences() override;

  private:
    std::unique_ptr<IPreferencesStorage> m_Storage;
  };
}

#endif
