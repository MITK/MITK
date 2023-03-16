/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIPreferencesStorage.h>

mitk::IPreferencesStorage::IPreferencesStorage(const std::filesystem::path& filename)
  : m_Filename(filename)
{
}

mitk::IPreferencesStorage::~IPreferencesStorage()
{
}

mitk::IPreferences* mitk::IPreferencesStorage::GetRoot()
{
  return m_Root.get();
}

const mitk::IPreferences* mitk::IPreferencesStorage::GetRoot() const
{
  return m_Root.get();
}

std::filesystem::path mitk::IPreferencesStorage::GetFilename() const
{
  return m_Filename;
}
