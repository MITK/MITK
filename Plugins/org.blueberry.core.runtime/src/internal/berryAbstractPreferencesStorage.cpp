/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "berryAbstractPreferencesStorage.h"

#include <berryIPreferences.h>

namespace berry
{

  AbstractPreferencesStorage::AbstractPreferencesStorage( const QString& _File )
    : m_File(_File)
    , m_Root(nullptr)
  {

  }

  AbstractPreferencesStorage::~AbstractPreferencesStorage()
  {

  }

  IPreferences::Pointer AbstractPreferencesStorage::GetRoot() const
  {
    return m_Root;
  }

  QString AbstractPreferencesStorage::GetFile() const
  {
    return m_File;
  }

  void AbstractPreferencesStorage::SetFile( const QString& f )
  {
    m_File = f;
  }
}
