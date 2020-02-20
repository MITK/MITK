/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQHELPENGINECONFIGURATION_H
#define BERRYQHELPENGINECONFIGURATION_H

#include <QMutex>

#include <ctkServiceRegistration.h>
#include <service/cm/ctkManagedService.h>


namespace berry {

class QHelpEngineWrapper;

class QHelpEngineConfiguration : public QObject, public ctkManagedService
{
  Q_OBJECT
  Q_INTERFACES(ctkManagedService)

public:

  QHelpEngineConfiguration(ctkPluginContext* context, QHelpEngineWrapper& helpEngine);

  void updated(const ctkDictionary &properties) override;

public Q_SLOTS:

  void configurationChanged(const ctkDictionary &properties);

private:

  Q_DISABLE_COPY(QHelpEngineConfiguration)

  ctkDictionary getDefaults() const;

  static QString PID;

  QMutex mutex;
  ctkServiceRegistration registration;

  QHelpEngineWrapper& helpEngine;
};

}

#endif // BERRYQHELPENGINECONFIGURATION_H
