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

#include "mitkCoreServicesPlugin.h"

#include "internal/mitkDataStorageService.h"

namespace mitk
{

const std::string CoreServicesPlugin::PLUGIN_ID = "org.mitk.core.services";

void CoreServicesPlugin::Start(berry::IBundleContext::Pointer context)
{
  DataStorageService::Pointer service(new DataStorageService());
  context->RegisterService(IDataStorageService::ID, service);
}

}
