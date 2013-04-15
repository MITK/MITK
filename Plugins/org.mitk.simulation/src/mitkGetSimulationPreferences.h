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

#ifndef mitkGetSimulationPreferences_h
#define mitkGetSimulationPreferences_h

#include <berryIPreferences.h>
#include <org_mitk_simulation_Export.h>

namespace mitk
{
  SIMULATION_INIT_EXPORT berry::IPreferences::Pointer GetSimulationPreferences();
}

#endif
