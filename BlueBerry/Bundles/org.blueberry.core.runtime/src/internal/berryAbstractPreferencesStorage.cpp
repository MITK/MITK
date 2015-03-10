/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "berryAbstractPreferencesStorage.h"

#include <berryIPreferences.h>

namespace berry
{

  AbstractPreferencesStorage::AbstractPreferencesStorage( const QString& _File )
    : m_File(_File)
    , m_Root(0)
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
