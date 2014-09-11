/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef QMITKXNATSESSIONMANAGER_H_
#define QMITKXNATSESSIONMANAGER_H_

#include <berryIPreferencesService.h>

#include "ctkXnatSession.h"
#include "ctkXnatLoginProfile.h"

class QmitkXnatSessionManager
{

public:

  QmitkXnatSessionManager();
  ~QmitkXnatSessionManager();

  /// \brief Opens a xnat session.
  ctkXnatSession* GetXnatSession();

  /// \brief Updates the xnat session.
  void UpdateXnatSession();

private:

  ctkXnatSession* m_Session;
  berry::IPreferencesService::WeakPtr m_PreferencesService;

};

#endif /*QMITKXNATSESSIONMANAGER_H_*/
