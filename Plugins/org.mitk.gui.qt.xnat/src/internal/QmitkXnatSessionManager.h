/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
  ctkXnatSession* m_Session;

};

#endif /*QMITKXNATSESSIONMANAGER_H_*/
