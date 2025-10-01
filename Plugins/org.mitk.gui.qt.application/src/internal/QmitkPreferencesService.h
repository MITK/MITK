/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPreferencesService_h
#define QmitkPreferencesService_h

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>

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
  mitk::CoreServicePointer<mitk::IPreferencesService> m_PreferencesService;
};

#endif
