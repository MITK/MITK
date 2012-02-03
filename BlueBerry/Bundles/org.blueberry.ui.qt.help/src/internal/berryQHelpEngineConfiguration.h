/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

  void updated(const ctkDictionary &properties);

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
