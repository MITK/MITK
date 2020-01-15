/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryQHelpEngineConfiguration.h"
#include "berryQHelpEngineWrapper.h"

#include <ctkPluginContext.h>


namespace berry {

QString QHelpEngineConfiguration::PID = "org.blueberry.services.help";

QHelpEngineConfiguration::QHelpEngineConfiguration(ctkPluginContext* context,
                                                   QHelpEngineWrapper& helpEngine)
  : helpEngine(helpEngine)
{
  QMutexLocker lock(&mutex);
  registration = context->registerService<ctkManagedService>(this, getDefaults());
}

void QHelpEngineConfiguration::updated(const ctkDictionary &properties)
{
  if (properties.isEmpty())
  {
    QMutexLocker lock(&mutex);
    registration.setProperties(getDefaults());
  }
  else
  {
    QMetaObject::invokeMethod(this, "configurationChanged", Q_ARG(ctkDictionary, properties));

    QMutexLocker lock(&mutex);
    registration.setProperties(properties);
  }
}

void QHelpEngineConfiguration::configurationChanged(const ctkDictionary& properties)
{
  helpEngine.setHomePage(properties["homePage"].toString());
}

ctkDictionary QHelpEngineConfiguration::getDefaults() const
{
  ctkDictionary defaults;
  defaults.insert("homePage", "");
  defaults.insert(ctkPluginConstants::SERVICE_PID, PID);
  return defaults;
}

}
