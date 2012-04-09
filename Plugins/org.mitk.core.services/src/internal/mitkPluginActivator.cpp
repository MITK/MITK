/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkPluginActivator.h"

#include "internal/mitkDataStorageService.h"

namespace mitk
{

const std::string org_mitk_core_services_Activator::PLUGIN_ID = "org.mitk.core.services";

void org_mitk_core_services_Activator::start(ctkPluginContext* context)
{
  DataStorageService* service = new DataStorageService();
  dataStorageService = IDataStorageService::Pointer(service);
  context->registerService<mitk::IDataStorageService>(service);
}

void org_mitk_core_services_Activator::stop(ctkPluginContext* /*context*/)
{
  dataStorageService = 0;
}

}

Q_EXPORT_PLUGIN2(org_mitk_core_services, mitk::org_mitk_core_services_Activator)
