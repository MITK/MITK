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

#include "ctkXnatLoginProfile.h"

#include "mitkXnatSessionTracker.h"

class QmitkXnatSessionManager
{

public:

  QmitkXnatSessionManager();
  ~QmitkXnatSessionManager();

  /// \brief Opens a xnat session.
  void OpenXnatSession();

  /// \brief Creates the xnat session.
  void CreateXnatSession();

  void CloseXnatSession();

  bool LastSessionIsValid();
  int AmountOfCreatedSessions();

private:

  us::ServiceRegistration<ctkXnatSession> m_SessionRegistration;
  berry::IPreferencesService::WeakPtr m_PreferencesService;
  ctkXnatSession* m_Session;

};

#endif /*QMITKXNATSESSIONMANAGER_H_*/
