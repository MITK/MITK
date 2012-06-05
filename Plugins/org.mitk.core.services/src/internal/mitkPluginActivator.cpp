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

#include "mitkPluginActivator.h"

#include "mitkLog.h"

#include <QString>
#include <QFileInfo>

#include "internal/mitkDataStorageService.h"

namespace mitk
{

const std::string org_mitk_core_services_Activator::PLUGIN_ID = "org.mitk.core.services";

void org_mitk_core_services_Activator::start(ctkPluginContext* context)
{
  //initialize logging
  mitk::LoggingBackend::Register(); 
  QString filename = "mitk.log";
  QFileInfo path = context->getDataFile(filename);
  mitk::LoggingBackend::SetLogFile(path.absoluteFilePath().toStdString().c_str());
  MITK_INFO << "Logfile: " << path.absoluteFilePath().toStdString().c_str();

  //initialize data storage service
  DataStorageService* service = new DataStorageService();
  dataStorageService = IDataStorageService::Pointer(service);
  context->registerService<mitk::IDataStorageService>(service);
}

void org_mitk_core_services_Activator::stop(ctkPluginContext* /*context*/)
{
  //clean up logging
  mitk::LoggingBackend::Unregister();
  
  dataStorageService = 0;
}

}

Q_EXPORT_PLUGIN2(org_mitk_core_services, mitk::org_mitk_core_services_Activator)
