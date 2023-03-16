/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPreferencesService.h"
#include "mitkXMLPreferencesStorage.h"

#include <mitkExceptionMacro.h>

mitk::PreferencesService::PreferencesService()
{
}

mitk::PreferencesService::~PreferencesService()
{
  if (m_Storage)
    m_Storage->Flush();
}

void mitk::PreferencesService::InitializeStorage(const std::filesystem::path& filename)
{
  if (m_Storage)
    mitkThrow() << "The preferences service must be initialized only once to guarantee valid preferences pointers during its lifetime.";

  m_Storage = std::make_unique<XMLPreferencesStorage>(filename);
}

void mitk::PreferencesService::UninitializeStorage(bool removeFile)
{
  if (m_Storage && removeFile)
    std::filesystem::remove(m_Storage->GetFilename());

  m_Storage.reset(nullptr);
}

mitk::IPreferences* mitk::PreferencesService::GetSystemPreferences()
{
  return m_Storage
    ? m_Storage->GetRoot()
    : nullptr;
}
