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

#include "mitkGetSimulationPreferences.h"
#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryServiceRegistry.h>

berry::IPreferences::Pointer mitk::GetSimulationPreferences()
{
  berry::ServiceRegistry& serviceRegistry = berry::Platform::GetServiceRegistry();
  berry::IPreferencesService::Pointer preferencesService = serviceRegistry.GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  berry::IPreferences::Pointer preferences = preferencesService->GetSystemPreferences();
  return preferences->Node("/org.mitk.views.simulation");
}
