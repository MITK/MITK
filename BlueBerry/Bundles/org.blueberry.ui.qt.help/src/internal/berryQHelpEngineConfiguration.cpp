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
