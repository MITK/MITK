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


#ifndef QMITKXNATCONNECTIONMANAGER_H_
#define QMITKXNATCONNECTIONMANAGER_H_

#include <QObject.h>
#include <berryIPreferencesService.h>

#include "ctkXnatSession.h"
#include "ctkXnatLoginProfile.h"

class QmitkXnatConnectionManager : public QObject
{
  Q_OBJECT

public:

  QmitkXnatConnectionManager();
  ~QmitkXnatConnectionManager();
  ctkXnatSession* GetXnatConnection();

private:

  ctkXnatSession* m_Session;
  berry::IPreferencesService::WeakPtr m_PreferencesService;

};

#endif /*QMITKXNATCONNECTIONMANAGER_H_*/
